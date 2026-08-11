"""
Creates the mod's input assets so the keys show up in Options > Keybindings.

Satisfactory only lists rebindable actions that come from cooked
FGChildInputMappingContext assets scanned by the mod's Game Feature Data, so these
cannot be built at runtime - hence generating them here rather than clicking
through the editor.

Run with:
    UnrealEditor-Cmd.exe <project>.uproject -run=pythonscript -script="<this file>"
"""

import unreal

PLUGIN = "PackHotkeys"
CONTENT_ROOT = "/{0}".format(PLUGIN)
INPUT_DIR = "{0}/Inputs".format(CONTENT_ROOT)
PARENT_CONTEXT = "/Game/FactoryGame/Inputs/Player/MC_PlayerActions"
CATEGORY = "Pack Hotkeys"

# asset name, default key, display name, description
ACTIONS = [
    (
        "IA_TogglePack",
        "F1",
        "Toggle Pack",
        "Equip the Jetpack, or swap between the Jetpack and the Hover Pack.",
    ),
    (
        "IA_EquipParachute",
        "F2",
        "Equip Parachute",
        "Equip the Parachute into the back slot.",
    ),
]

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

# Without this the registry does not know about assets left over from a previous run,
# so does_asset_exist says no while asset creation then refuses to overwrite them.
unreal.AssetRegistryHelpers.get_asset_registry().scan_paths_synchronous(
    [CONTENT_ROOT], force_rescan=True
)


def create_asset(name, package_path, asset_class, factory):
    full_path = "{0}/{1}".format(package_path, name)
    if unreal.EditorAssetLibrary.does_asset_exist(full_path):
        unreal.log("Deleting existing {0}".format(full_path))
        unreal.EditorAssetLibrary.delete_asset(full_path)
    asset = asset_tools.create_asset(name, package_path, asset_class, factory)
    if asset is None:
        raise RuntimeError("could not create {0}".format(full_path))
    return asset


def data_asset_factory(asset_class):
    factory = unreal.DataAssetFactory()
    factory.set_editor_property("supported_class", asset_class)
    return factory


def make_action(asset_name, display_name, description):
    action = create_asset(
        asset_name,
        INPUT_DIR,
        unreal.InputAction,
        unreal.InputAction_Factory(),
    )
    action.set_editor_property("value_type", unreal.InputActionValueType.BOOLEAN)
    action.set_editor_property("action_description", unreal.Text(description))
    # Never swallow the key - other mods and the base game may want it too.
    action.set_editor_property("consume_input", False)

    # This object is what makes the action show up in the keybinding menu, and as of 1.2
    # it lives on the action rather than on the context mapping.
    # It has to be created as a subobject of the action: a plain constructor call produces
    # a transient object that is silently dropped when the asset is saved.
    settings = unreal.new_object(
        unreal.PlayerMappableKeySettings,
        outer=action,
        name="PlayerMappableKeySettings_0",
    )
    settings.set_editor_property(
        "name", unreal.Name("{0}_{1}".format(PLUGIN, asset_name.replace("IA_", "")))
    )
    settings.set_editor_property("display_name", unreal.Text(display_name))
    settings.set_editor_property("display_category", unreal.Text(CATEGORY))
    action.set_editor_property("player_mappable_key_settings", settings)

    unreal.EditorAssetLibrary.save_loaded_asset(action)
    return action


def make_context(actions_with_keys):
    context = create_asset(
        "MC_{0}".format(PLUGIN),
        INPUT_DIR,
        unreal.FGChildInputMappingContext,
        data_asset_factory(unreal.FGChildInputMappingContext),
    )
    # Parenting to MC_PlayerActions makes the game apply and remove our bindings
    # alongside the normal player action context, including while menus are open.
    # The property is a TSoftObjectPtr, but Python wants the resolved object here.
    parent = unreal.EditorAssetLibrary.load_asset(PARENT_CONTEXT)
    if parent is None:
        raise RuntimeError("parent context {0} not found".format(PARENT_CONTEXT))
    context.set_editor_property("mParentContext", parent)
    context.set_editor_property("mDisplayName", unreal.Text(CATEGORY))
    # Below 200 would sort this category in among the base game ones.
    context.set_editor_property("mMenuPriority", 200.0)

    mappings = []
    for action, key_name in actions_with_keys:
        key = unreal.Key()
        key.set_editor_property("key_name", unreal.Name(key_name))

        mapping = unreal.EnhancedActionKeyMapping()
        mapping.set_editor_property("action", action)
        mapping.set_editor_property("key", key)
        mappings.append(mapping)
    context.set_editor_property("mappings", mappings)

    unreal.EditorAssetLibrary.save_loaded_asset(context)
    return context


def make_game_feature_data(context_path):
    data = create_asset(
        PLUGIN,
        CONTENT_ROOT,
        unreal.FGGameFeatureData,
        data_asset_factory(unreal.FGGameFeatureData),
    )

    type_info = unreal.PrimaryAssetTypeInfo()
    type_info.set_editor_property("primary_asset_type", unreal.Name("FGChildInputMappingContext"))
    type_info.set_editor_property("asset_base_class", unreal.FGChildInputMappingContext)
    # bHasBlueprintClasses / bIsEditorOnly stay at their defaults of false.
    type_info.set_editor_property("specific_assets", [unreal.SoftObjectPath(context_path)])
    data.set_editor_property("primary_asset_types_to_scan", [type_info])

    unreal.EditorAssetLibrary.save_loaded_asset(data)
    return data


def run():
    actions_with_keys = []
    for asset_name, key, display_name, description in ACTIONS:
        action = make_action(asset_name, display_name, description)
        actions_with_keys.append((action, key))
        unreal.log("PACKHOTKEYS created {0} default {1}".format(asset_name, key))

    make_context(actions_with_keys)
    unreal.log("PACKHOTKEYS created MC_{0}".format(PLUGIN))

    make_game_feature_data("{0}/MC_{1}.MC_{1}".format(INPUT_DIR, PLUGIN))
    unreal.log("PACKHOTKEYS created game feature data")


run()
