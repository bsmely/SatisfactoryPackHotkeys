# Pack Hotkeys

Separate hotkeys for the back equipment slot in Satisfactory 1.2: one key each for the
**Jetpack**, the **Hover Pack** and the **Parachute**, instead of opening the inventory
and dragging items around.

Defaults: `F1` Jetpack, `F2` Hover Pack, `F` Parachute.

> [!NOTE]
> **Build status:** compiles clean against UE 5.6.1-CSS / SML 3.12 and packages into a
> loadable Shipping build via Alpakit. In-game behaviour is still being verified — see
> [Known limitations](#known-limitations) for the parts most likely to need work.

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

The whole cycle also runs headless, without opening the editor:

```powershell
$engine = 'D:\Dev\Tools\ue-css'
$project = 'D:\Dev\Tools\sml\FactoryGame.uproject'

# project files (once)
& "$engine\Engine\Build\BatchFiles\Build.bat" -projectfiles -project="$project" -game -rocket -progress

# editor target (once, ~10 min; also applies the WwisePatches pre-build steps)
& "$engine\Engine\Build\BatchFiles\Build.bat" FactoryEditor Win64 Development -project="$project" -progress

# build + cook + package the mod, optionally straight into the game
& "$engine\Engine\Build\BatchFiles\RunUAT.bat" -ScriptsForProject="$project" PackagePlugin `
    -project="$project" -clientconfig=Shipping -serverconfig=Shipping -utf8output `
    -DLCName=PackHotkeys -build -platform=Win64 -nocompileeditor `
    -CopyToGameDirectory_Windows="<path to Satisfactory>"
```

Note the editor target is called `FactoryEditor`, not `FactoryGameEditor`. The output archive
lands in `<StarterProject>/Saved/ArchivedPlugins/PackHotkeys/PackHotkeys-Windows.zip`; to
install by hand, unzip it into `<Satisfactory>/FactoryGame/Mods/PackHotkeys`.

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
