# SimCopter reconstruction architecture

## Evidence captured

The supplied disc contains a 32-bit Windows PE executable with SHA-256
`a20bc20616f705df9d2a700c397704e93aea087c1167d54137fa4efaddf392b4`.
Ghidra 12.1.4 with GhidraMCP 7.0.0 identified 3,050 functions, 12,510
symbols, 260 imports, and 1,149 defined strings.

The original executable uses Win32, DirectDraw, DirectSound, WinMM joystick
and timer APIs, Glide, and Smacker video. Its primary game-resource bootstrap
at `0x0047de90` loads `sim3d.twk`, `SIM3D.BMP`, and three `SIM3D*.MAX`
geometry archives before initializing gameplay systems.

The tuning loader at `0x004667f0` opens `sim3d.twk`, looks up each registered
section's `Redirect` key, and delegates to the corresponding subsystem `.twk`
file. The supplied tuning files expose helicopter flight, landing, rope,
damage, fire, camera, figure, career, and automatic-mission parameters.

The mission registration routine at `0x004b1bd0` establishes nine families:

1. General
2. Riot
3. Rescue
4. Transport
5. Medevac
6. Fire
7. Criminal
8. Speeder
9. Traffic

The geometry path is now decoded. `InitializeGameResources` at `0x0047de90`
opens `SIM3D1.MAX`, `SIM3D2.MAX`, and `SIM3D3.MAX` through the archive loader at
`0x0046e4b0`. The DIRC/GEOM parser at `0x0046ec40` indexes named records, while
the object decoder at `0x00470c00` reads `OBJX` vertices and variable-length
`FACE` records. Despite their filenames, these are Maxis DIRC archives rather
than Autodesk 3ds Max files. `tools/extract_sim3d_models.py` implements that
recovered format and produces deterministic OBJ/MTL intermediates.

## Unreal mapping

- `AHelicopterPawn` owns the flight model and chase camera. Its body and rotor
  components now use the original `BELL212` and `BELLROTR` geometry; its public
  tuning struct is the first replacement for the original redirected TWK
  system.
- `AMissionDirector` owns dispatch and mission lifecycle. The first vertical
  slice cycles through all nine Ghidra-confirmed mission families.
- `AProceduralCity` supplies a deterministic flyable city using eight recovered
  original structure/landmark models while the SC2 city-layout format is
  decoded.
- `ASimCopterHUD` combines a resolution-independent flight readout with the
  authorized original dashboard strip imported through Unreal.
- `ASimCopterGameMode` assembles the playable sandbox without requiring any
  editor-created Blueprint assets.

## Next reverse-engineering targets

1. Decode the texture-addressing rules and bitmap container used by `FACE`
   records marked with flag `0x2000`; flat archive-palette faces are complete.
2. Decode SC2 IFF city chunks and map zones, roads, terrain, and buildings to
   procedural Unreal actors.
3. Trace the mission update functions reached from the registration tables to
   recover scoring, spawning, timeouts, and completion conditions.
4. Expand the working BMP/WAV import pipeline beyond the dashboard and rotor /
   mission sounds to the remaining authorized UI, speech, and effects.
5. Recover the remaining aircraft variants and map the nine `heli.twk` profiles
   to data assets.
