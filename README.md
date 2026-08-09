# Pack Hotkeys

Separate hotkeys for the back equipment slot in Satisfactory 1.2: one key each for the
**Jetpack**, the **Hover Pack** and the **Parachute**, instead of opening the inventory
and dragging items around.

Defaults: `F1` Jetpack, `F2` Hover Pack, `F` Parachute.

> [!WARNING]
> **This mod has never been compiled or run.** It was written against the Satisfactory 1.2
> headers from the Starter Project, but the author has no modding environment set up, so there
> is no packaged build and no in-game testing behind it. Treat it as a starting point that
> almost certainly needs a fix or two on first compile. If you build it and it works — or
> doesn't — please open an issue and say so.

## What it does

Pressing a key equips the matching pack into the back equipment slot:

1. If that kind of pack is already equipped, nothing happens (or it is put away, if
   `bUnequipWhenAlreadyEquipped` is enabled).
2. If the pack already sits in the back slot inventory, it is simply made the active one.
3. Otherwise the first matching item in the player inventory is moved into the back slot,
   swapping out whatever was there.

Matching is done on the native equipment classes `AFGJetPack`, `AFGHoverPack` and
`AFGParachute`, so modded packs deriving from them are picked up as well.

## Configuration

The mod is pure C++ with no Blueprint content, which means it cannot use the in-game mod
settings menu. Keys are read from `Game.ini`:

`<Satisfactory>/FactoryGame/Saved/Config/Windows/Game.ini`

```ini
[/Script/PackHotkeys.PackHotkeysSettings]
JetPackKey=(Key=F1)
HoverPackKey=(Key=F2)
ParachuteKey=(Key=F)
bUnequipWhenAlreadyEquipped=False
MappingContextPriority=1000
```

Raise `MappingContextPriority` if another mod or the base game claims one of the keys.

## Multiplayer

Inventory moves are authority-side, so the mod talks to the server through a Remote Call
Object. In single player and as the host of a listen server this is all local and works
as-is.

On a **dedicated server** the mod currently will not work: the RCO is registered from the
local player's input setup, which never runs on a headless server. Fixing that properly
needs a Game Instance Module asset, which in turn needs the Unreal editor — see
[Known limitations](#known-limitations).

## Known limitations

- **Keys are not in the game's keybinding menu.** Satisfactory discovers rebindable actions
  through cooked `UFGChildInputMappingContext` assets scanned by the Asset Manager, and this
  mod creates its input actions and mapping context at runtime instead. Rebinding happens in
  the ini file. Adding a proper asset would make the keys appear under Options → Keybindings.
- **Dedicated servers are unsupported**, as described above.
- The mapping context is registered directly on the Enhanced Input subsystem rather than
  through Satisfactory's parent-context system (again, that needs assets). To avoid firing
  while the player is typing, the mod ignores key presses when a text field has keyboard
  focus or the pause menu is open.

## Building

You need the Satisfactory modding environment: Unreal Engine 5.6.1-CSS, Visual Studio 2022,
Wwise and the Starter Project. Follow the
[official setup guide](https://docs.ficsit.app/satisfactory-modding/latest/Development/BeginnersGuide/index.html).

Then:

1. Clone this repository into `<StarterProject>/Mods/PackHotkeys` — the folder name must
   match the plugin name.
2. Regenerate project files and build the editor target.
3. Package with Alpakit (`Alpakit Dev` installs it straight into your game).

## Compatibility

- Satisfactory 1.2, CL 491125 or newer
- SML `^3.12.0`

## Credit

The approach — resolving an item descriptor to its equipment class and moving it into the
back slot — follows what [Togglepack](https://ficsit.app/mod/CXCwQ2ShNA1w1x) by derpierre65
does for its jetpack/hover pack toggle. The code here is an independent C++ implementation,
not a copy of that mod's Blueprints.

## License

MIT, see [LICENSE](LICENSE).
