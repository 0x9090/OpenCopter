import unreal


LEVEL_PATH = "/Game/Maps/SimCopterCity"


level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

if unreal.EditorAssetLibrary.does_asset_exist(LEVEL_PATH):
    unreal.log(f"Level already exists: {LEVEL_PATH}")
else:
    if not level_editor.new_level(LEVEL_PATH, False):
        raise RuntimeError(f"Unable to create level: {LEVEL_PATH}")
    unreal.log(f"Created level: {LEVEL_PATH}")

if not unreal.EditorAssetLibrary.save_asset(LEVEL_PATH, only_if_is_dirty=False):
    raise RuntimeError(f"Unable to save level: {LEVEL_PATH}")

unreal.log(f"Saved level: {LEVEL_PATH}")
