# Original disc inventory

The Nero image contains an ISO-9660 session labeled `SimCopter`. It was
extracted with 7-Zip 26.00 into `ReverseEngineering/disc/`.

## Primary game payload

- `SIMCOPTER/SimCopter.exe`: 1,569,280-byte PE32 i386 Windows GUI executable.
- SHA-256: `a20bc20616f705df9d2a700c397704e93aea087c1167d54137fa4efaddf392b4`.
- `SIMCOPTER/geo/SIM3D1.MAX` through `SIM3D3.MAX`: DIRC/CMAP/GEOM resource archives.
- `SIMCOPTER/cities/*.SC2`: SimCity 2000 IFF city files.
- `SIMCOPTER/tweak/*.TWK`: readable gameplay tuning tables.
- `SIMCOPTER/x/*.DF`: people and private-animation data.

## Asset counts under `SIMCOPTER/`

| Count | Extension | Likely role |
|---:|---|---|
| 507 | WAV | effects, speech, dispatch, radio |
| 181 | BMP | interface, palettes, textures |
| 97 | GIF | HTML help imagery |
| 75 | SMK | Smacker cinematics and animated panels |
| 54 | SC2 | supplied cities |
| 45 | HTM | in-game help |
| 8 | TWK | gameplay tuning |
| 8 | DEF | resource definitions |
| 3 | MAX | geometry/resource archives |
| 2 | SCU | user-game state |
| 2 | DF | people and animation data |

The largest content groups are `smk/` (258 MiB), `sound/` (210 MiB), `bmp/`
(11 MiB), `help/` (6.2 MiB), and `cities/` (4.4 MiB).

## Ghidra summary

- Architecture: x86 little-endian, 32-bit, Windows compiler specification.
- Image base: `0x00400000`; entry point: `0x004dcf20`.
- Auto-analysis: 3,050 functions, 12,510 symbols.
- Imports: 260 across KERNEL32, USER32, GDI32, COMDLG32, ADVAPI32, WINMM,
  GLIDE, DDRAW, DSOUND, MSACM32, VERSION, and SMACKW32.
- Defined strings: 1,149.

Machine-readable inventories and selected decompilations are stored beside
this report as JSON.
