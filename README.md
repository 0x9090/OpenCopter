# SimCopter Unreal reconstruction

This workspace now contains an evidence-driven Unreal Engine 5.8 vertical
slice and a preserved reverse-engineering corpus for the supplied SimCopter
disc image.

## Play the current slice

Open `SimCopterRemake.uproject` in Unreal Engine 5.8 and press Play. The game
starts in `/Game/Maps/SimCopterCity` and creates the city, helicopter, lighting,
HUD, and dispatch system at runtime. It opens on a reconstructed title screen
using the original SimCopter menu and hangar art. Start Career, Free Flight,
Load Game, Controls, and Quit are available from that screen.

Controls:

- `W` / `S`: forward / reverse pitch
- `A` / `D`: strafe and visual bank
- `Q` / `E`: yaw
- `Space` / `Left Ctrl`: climb / descend
- `Esc`: open or close the in-flight menu
- `F5`: quick save
- `F9`: quick load
- `Enter`: start a career from the title screen or resume from pause

The vertical slice imports the original dashboard pieces, compass, pilot
portrait sheet, day/night hangar art, Bell 212 catalog art, building and ground
textures, plus selected rotor/mission/menu audio from the authorized disc image
into `/Game/Original`. The runtime city applies the recovered building and
ground artwork to its generated structures, and the HUD reconstructs the
original cockpit frame around the modern mission readout.
Re-run `tools/import_original_assets.py` through Unreal Editor if the source
disc extraction is refreshed.

Approach the red mission beacon at low speed to complete a call. The mission
director cycles through the nine mission families recovered from the original
executable.

## Build from the command line

```sh
"/Users/Shared/Epic Games/UE_5.8/Engine/Build/BatchFiles/Mac/Build.sh" \
  SimCopterRemakeEditor Mac Development \
  "/Users/nops/Code/Sim Copter/SimCopterRemake.uproject" -WaitMutex
```

## Reverse-engineering workspace

The disc extraction, Ghidra project, MCP captures, decompilations, and reports
are under `ReverseEngineering/`. Start the workspace-local GhidraMCP headless
server with:

```sh
./tools/start-ghidra-mcp.sh
```

The original binary remains untouched. Ghidra annotations are stored in
`ReverseEngineering/ghidra-projects/SimCopter`.

See `ReverseEngineering/reports/architecture.md` for the recovered subsystem
map and the next reverse-engineering targets.
