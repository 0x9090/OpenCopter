import os
import unreal


ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
DISC = os.path.join(ROOT, "ReverseEngineering", "disc", "SIMCOPTER")
REPORTS = os.path.join(ROOT, "ReverseEngineering", "reports")


def import_asset(source, destination_path, destination_name):
    if not os.path.isfile(source):
        raise RuntimeError(f"Missing source asset: {source}")

    task = unreal.AssetImportTask()
    task.filename = source
    task.destination_path = destination_path
    task.destination_name = destination_name
    task.automated = True
    task.replace_existing = True
    task.replace_existing_settings = True
    task.save = True
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

    if not task.imported_object_paths:
        raise RuntimeError(f"Unreal did not import {source}")
    unreal.log(f"Imported {source} -> {task.imported_object_paths}")
    return task.imported_object_paths[0]


def replace_material(name):
    path = f"/Game/Original/Materials/{name}"
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        unreal.EditorAssetLibrary.delete_asset(path)
    return unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        name,
        "/Game/Original/Materials",
        unreal.Material,
        unreal.MaterialFactoryNew(),
    )


def make_textured_material(name, texture_path, uv_scale=1.0):
    material = replace_material(name)
    material.set_editor_property("two_sided", True)
    material.set_editor_property("used_with_instanced_static_meshes", True)

    sample = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionTextureSample, -100, 0
    )
    sample.texture = unreal.load_asset(texture_path)

    if uv_scale != 1.0:
        coordinates = unreal.MaterialEditingLibrary.create_material_expression(
            material, unreal.MaterialExpressionTextureCoordinate, -600, 120
        )
        coordinates.set_editor_property("u_tiling", uv_scale)
        coordinates.set_editor_property("v_tiling", uv_scale)
        unreal.MaterialEditingLibrary.connect_material_expressions(coordinates, "", sample, "Coordinates")

    unreal.MaterialEditingLibrary.connect_material_property(
        sample, "RGB", unreal.MaterialProperty.MP_BASE_COLOR
    )
    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material, only_if_is_dirty=False)


def make_color_key_ui_material(name, texture_path):
    material = replace_material(name)
    material.set_editor_property("material_domain", unreal.MaterialDomain.MD_UI)
    material.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)

    sample = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionTextureSample, -700, 0
    )
    sample.texture = unreal.load_asset(texture_path)

    one = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionConstant, -700, 310
    )
    one.set_editor_property("r", 1.0)
    one_minus_red = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionSubtract, -480, 190
    )
    green_blue = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionMultiply, -480, 280
    )
    cyan_match = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionMultiply, -260, 230
    )
    opacity = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionSubtract, -50, 230
    )

    unreal.MaterialEditingLibrary.connect_material_expressions(one, "", one_minus_red, "A")
    unreal.MaterialEditingLibrary.connect_material_expressions(sample, "R", one_minus_red, "B")
    unreal.MaterialEditingLibrary.connect_material_expressions(sample, "G", green_blue, "A")
    unreal.MaterialEditingLibrary.connect_material_expressions(sample, "B", green_blue, "B")
    unreal.MaterialEditingLibrary.connect_material_expressions(green_blue, "", cyan_match, "A")
    unreal.MaterialEditingLibrary.connect_material_expressions(one_minus_red, "", cyan_match, "B")
    unreal.MaterialEditingLibrary.connect_material_expressions(one, "", opacity, "A")
    unreal.MaterialEditingLibrary.connect_material_expressions(cyan_match, "", opacity, "B")
    unreal.MaterialEditingLibrary.connect_material_property(
        sample, "RGB", unreal.MaterialProperty.MP_EMISSIVE_COLOR
    )
    unreal.MaterialEditingLibrary.connect_material_property(
        opacity, "", unreal.MaterialProperty.MP_OPACITY
    )
    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material, only_if_is_dirty=False)


unreal.EditorAssetLibrary.make_directory("/Game/Original/Audio")
unreal.EditorAssetLibrary.make_directory("/Game/Original/UI")
unreal.EditorAssetLibrary.make_directory("/Game/Original/World")
unreal.EditorAssetLibrary.make_directory("/Game/Original/Materials")

import_asset(
    os.path.join(DISC, "sound", "COPLOOP.WAV"),
    "/Game/Original/Audio",
    "S_RotorLoop",
)
import_asset(os.path.join(DISC, "sound", "menu.wav"), "/Game/Original/Audio", "S_MenuSelect")
import_asset(os.path.join(DISC, "sound", "menuback.wav"), "/Game/Original/Audio", "S_MenuBack")
import_asset(
    os.path.join(DISC, "sound", "CHOPSTAR.WAV"),
    "/Game/Original/Audio",
    "S_RotorStart",
)
import_asset(
    os.path.join(DISC, "sound", "CA_CHING.WAV"),
    "/Game/Original/Audio",
    "S_MissionComplete",
)
import_asset(
    os.path.join(REPORTS, "original-dash6.png"),
    "/Game/Original/UI",
    "T_OriginalDashboard",
)

ui_assets = {
    "T_MainMenu": "main1.bmp",
    "T_PlayMenu": "playmenu.bmp",
    "T_HangarDay": "nhangar.bmp",
    "T_HangarNight": "dhangar.bmp",
    "T_DashboardSide": "dash2.bmp",
    "T_DashboardTop": "dash4.bmp",
    "T_DashboardRadio": "dash5.bmp",
    "T_DashboardGauges": "dash6.bmp",
    "T_Compass": "compass1.bmp",
    "T_PilotPortraits": "people1.bmp",
    "T_Bell212": "cat_bell.bmp",
}
for asset_name, filename in ui_assets.items():
    import_asset(os.path.join(DISC, "bmp", filename), "/Game/Original/UI", asset_name)

import_asset(
    os.path.join(DISC, "bmp", "bldghitx.bmp"),
    "/Game/Original/World",
    "T_OriginalBuilding",
)
import_asset(
    os.path.join(DISC, "bmp", "grndhitx.bmp"),
    "/Game/Original/World",
    "T_OriginalGround",
)

make_textured_material(
    "M_OriginalGround", "/Game/Original/World/T_OriginalGround", uv_scale=48.0
)
make_textured_material(
    "M_OriginalBuilding", "/Game/Original/World/T_OriginalBuilding", uv_scale=1.0
)

for asset_name in ui_assets:
    make_color_key_ui_material(
        f"M_{asset_name[2:]}", f"/Game/Original/UI/{asset_name}"
    )
make_color_key_ui_material(
    "M_DashboardLower", "/Game/Original/UI/T_OriginalDashboard"
)

unreal.EditorAssetLibrary.save_directory("/Game/Original", only_if_is_dirty=False, recursive=True)
unreal.log("SIMCOPTER_ORIGINAL_ASSET_IMPORT_COMPLETE")
