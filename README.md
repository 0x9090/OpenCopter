# SimCopter Unreal reconstruction

This workspace now contains an evidence-driven Unreal Engine 5.8 vertical
slice and a preserved reverse-engineering corpus for the supplied SimCopter
disc image.

## Play the current slice

Open `SimCopterRemake.uproject` in Unreal Engine 5.8 and press Play. The game
starts in `/Game/Maps/SimCopterCity` and creates the city, helicopter, lighting,
HUD, and dispatch system at runtime. It opens on the original 640x480 menu
composition recovered from `menusky.smk` and `main1.bmp` through `main5.bmp`.
New/Open Career, New/Open User Game, and Quit are available from that screen.

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
into `/Game/Original`. The helicopter body and rotor are the original `BELL212`
and `BELLROTR` meshes decoded from `SIM3D2.MAX`. The generated city now places
eight original structure and landmark meshes decoded from `SIM3D1.MAX` and
`SIM3D3.MAX`, with their archive palette materials. Those palette bytes are
converted from sRGB to Unreal linear color and rendered with a rough,
directionally lit surface, retaining the source model definition without the
former missing-material silhouettes. The HUD uses
the original dashboard pieces at their recovered 640x480 screen coordinates.

To reproduce the recovered model assets after refreshing the disc extraction:

```sh
python3 tools/extract_sim3d_models.py \
  ReverseEngineering/disc/SIMCOPTER/geo/sim3d1.max \
  ReverseEngineering/disc/SIMCOPTER/geo/sim3d2.max \
  ReverseEngineering/disc/SIMCOPTER/geo/sim3d3.max \
  --output Intermediate/RecoveredModels \
  --models BELL212 BELLROTR CO182 AP230 AR254 ST215 IN191 PP204 PK238 ML239

"/Users/Shared/Epic Games/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd" \
  SimCopterRemake.uproject \
  -ExecutePythonScript="$(pwd)/tools/import_original_models.py" \
  -unattended -nop4 -nosplash -nullrhi
```

Re-run `tools/import_original_assets.py` through Unreal Editor when the other
source artwork or audio extraction is refreshed.

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
map and `ReverseEngineering/reports/sim3d-model-format.md` for the decoded
model archive format and current asset inventory.
