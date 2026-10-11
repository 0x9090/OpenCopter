"""Import geometry extracted from SIM3D*.MAX into the Unreal project."""

import json
import os
import re
import unreal


ROOT = os.path.dirname(unreal.Paths.get_project_file_path())
SOURCE_DIR = os.path.join(ROOT, "Intermediate", "RecoveredModels")
MANIFEST_PATH = os.path.join(SOURCE_DIR, "manifest.json")
DESTINATION = "/Game/Original/Models"
PALETTE_MATERIALS = {}


def srgb_byte_to_linear(value):
    channel = value / 255.0
    if channel <= 0.04045:
        return channel / 12.92
    return ((channel + 0.055) / 1.055) ** 2.4


def make_palette_material(archive_name, palette_index, rgb):
    archive_stem = os.path.splitext(archive_name)[0].upper()
    asset_name = f"M_{archive_stem}_Palette_{palette_index}"
    asset_path = f"{DESTINATION}/{asset_name}"
    if asset_path in PALETTE_MATERIALS:
        return PALETTE_MATERIALS[asset_path]
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        unreal.EditorAssetLibrary.delete_asset(asset_path)
    material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name,
        DESTINATION,
        unreal.Material,
        unreal.MaterialFactoryNew(),
    )
    is_color_key = archive_stem == "SIM3D2" and palette_index == "006"
    if is_color_key:
        # BELLROTR's 0x0002 faces use the archive's green key (index 6) for
        # the spinning-disc interior. The original renderer discarded the
        # key; drawing it opaque creates a fluorescent disk above the Bell.
        material.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
        opacity = unreal.MaterialEditingLibrary.create_material_expression(
            material, unreal.MaterialExpressionConstant, -300, 260
        )
        opacity.set_editor_property("r", 0.0)
        unreal.MaterialEditingLibrary.connect_material_property(
            opacity, "", unreal.MaterialProperty.MP_OPACITY
        )
    color = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionConstant3Vector, -300, 0
    )
    color.set_editor_property(
        "constant",
        unreal.LinearColor(
            srgb_byte_to_linear(rgb[0]),
            srgb_byte_to_linear(rgb[1]),
            srgb_byte_to_linear(rgb[2]),
            1.0,
        ),
    )
    unreal.MaterialEditingLibrary.connect_material_property(
        color, "", unreal.MaterialProperty.MP_BASE_COLOR
    )
    roughness = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionConstant, -300, 140
    )
    roughness.set_editor_property("r", 0.88)
    unreal.MaterialEditingLibrary.connect_material_property(
        roughness, "", unreal.MaterialProperty.MP_ROUGHNESS
    )

    material.set_editor_property("used_with_instanced_static_meshes", True)
    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material, only_if_is_dirty=False)
    PALETTE_MATERIALS[asset_path] = material
    return material


if not os.path.isfile(MANIFEST_PATH):
    raise RuntimeError(
        "Missing recovered-model manifest. Run tools/extract_sim3d_models.py first."
    )

with open(MANIFEST_PATH, "r", encoding="utf-8") as manifest_file:
    manifest = json.load(manifest_file)

unreal.EditorAssetLibrary.make_directory(DESTINATION)

tasks = []
for item in manifest:
    source = os.path.abspath(os.path.join(ROOT, item["obj"]))
    if not os.path.isfile(source):
        raise RuntimeError(f"Missing recovered OBJ: {source}")

    options = unreal.FbxImportUI()
    options.set_editor_property("import_mesh", True)
    options.set_editor_property("import_as_skeletal", False)
    options.set_editor_property("import_materials", True)
    options.set_editor_property("import_textures", False)
    static_options = options.get_editor_property("static_mesh_import_data")
    static_options.set_editor_property("combine_meshes", True)
    static_options.set_editor_property("generate_lightmap_u_vs", True)
    static_options.set_editor_property("auto_generate_collision", False)

    task = unreal.AssetImportTask()
    task.set_editor_property("filename", source)
    task.set_editor_property("destination_path", DESTINATION)
    task.set_editor_property("destination_name", item["name"])
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("replace_existing_settings", True)
    task.set_editor_property("save", True)
    task.set_editor_property("options", options)
    tasks.append(task)

unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)

generated_instances = set()
for task, item in zip(tasks, manifest):
    mesh_path = f"{DESTINATION}/{item['name']}"
    mesh = unreal.load_asset(mesh_path)
    if not isinstance(mesh, unreal.StaticMesh):
        raise RuntimeError(f"Unreal did not create the expected static mesh: {mesh_path}")
    body_setup = mesh.get_editor_property("body_setup")
    if body_setup:
        body_setup.set_editor_property(
            "collision_trace_flag", unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE
        )

    # Interchange creates MaterialInstanceConstant assets whose engine parent
    # cannot persist project-specific ISM usage. Replace them with project-owned
    # palette materials compiled for instancing, preserving every material slot.
    for slot_index, static_material in enumerate(
        mesh.get_editor_property("static_materials")
    ):
        imported_material = static_material.get_editor_property("material_interface")
        if imported_material:
            generated_instances.add(imported_material.get_path_name().split(".", 1)[0])
        slot_name = str(static_material.get_editor_property("material_slot_name"))
        match = re.search(r"_Palette_(\d{3})$", slot_name)
        if not match:
            raise RuntimeError(f"Unexpected recovered material slot: {slot_name}")
        palette_index = match.group(1)
        palette_material = make_palette_material(
            item["archive"], palette_index, item["palette"][palette_index]
        )
        mesh.set_material(slot_index, palette_material)
    unreal.EditorAssetLibrary.save_loaded_asset(mesh, only_if_is_dirty=False)
    unreal.log(
        f"Recovered {item['name']}: {item['vertices']} vertices, "
        f"{item['faces']} faces from {item['archive']}"
    )

for item in manifest:
    for palette_index in item["palette"]:
        generated_instances.add(
            f"{DESTINATION}/{item['name']}_Palette_{palette_index}"
        )

for asset_path in sorted(generated_instances):
    asset_name = asset_path.rsplit("/", 1)[-1]
    if (
        "/Original/Models/" in asset_path
        and "_Palette_" in asset_path
        and not asset_name.startswith("M_")
        and unreal.EditorAssetLibrary.does_asset_exist(asset_path)
    ):
        unreal.EditorAssetLibrary.delete_asset(asset_path)

# Interchange can finish replacing its generated material instances at the end
# of an import task. Re-apply the project-owned palette materials after those
# temporary assets are gone so the saved mesh never retains a stale instance
# reference (which renders black when the instance package is deleted).
for item in manifest:
    mesh_path = f"{DESTINATION}/{item['name']}"
    mesh = unreal.load_asset(mesh_path)
    for slot_index, static_material in enumerate(
        mesh.get_editor_property("static_materials")
    ):
        slot_name = str(static_material.get_editor_property("material_slot_name"))
        match = re.search(r"_Palette_(\d{3})$", slot_name)
        if not match:
            raise RuntimeError(f"Unexpected recovered material slot: {slot_name}")
        palette_index = match.group(1)
        mesh.set_material(
            slot_index,
            make_palette_material(
                item["archive"], palette_index, item["palette"][palette_index]
            ),
        )
    unreal.EditorAssetLibrary.save_loaded_asset(mesh, only_if_is_dirty=False)

unreal.EditorAssetLibrary.save_directory(
    DESTINATION, only_if_is_dirty=False, recursive=True
)
unreal.log("SIMCOPTER_ORIGINAL_MODEL_IMPORT_COMPLETE")
