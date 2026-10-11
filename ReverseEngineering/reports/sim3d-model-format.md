# SIM3D model archive recovery

SimCopter's `SIM3D1.MAX`, `SIM3D2.MAX`, and `SIM3D3.MAX` files are proprietary
Maxis DIRC archives, not Autodesk 3ds Max scenes. This layout was recovered by
following the original executable's resource initialization and geometry
decoder in Ghidra.

## Executable evidence

- `0x0047de90` initializes the three archives plus `SIM3D.BMP` and `sim3d.twk`.
- `0x0046e4b0` opens an archive and dispatches its resource chunks.
- `0x0046ec40` parses the `DIRC` chunk table and 0x35-byte named `GEOM` entries.
- `0x00470a20` allocates and dispatches geometry objects.
- `0x00470c00` decodes `OBJX` and its trailing `FACE` records.

An archive starts with `DIRC`, a chunk count, and `(tag, offset)` pairs. `CMAP`
contains the archive's 256-entry RGB palette. `GEOM` points to the named-object
directory. Each ordinary object points to `OBJX`; aggregate entries reference
multiple parts and are deliberately rejected by the first-pass exporter.

`OBJX` contains a 124-byte object header, a 12-byte pivot/translation, signed
16.16 fixed-point vertices, then variable-length `FACE` records. A face stores
its corner count, flags/material fields, one-based 16-bit vertex indices, and
one signed 16.16 UV pair per corner. Some `FACE` records represent lines or
points; the exporter keeps their vertices but omits them from Unreal's polygon
mesh. Coordinates are converted from Maxis X-right/Y-up/Z-forward into Unreal
X-forward/Y-right/Z-up and scaled at 40 cm per source unit. This makes the
recovered Bell 212 approximately 17.7 metres long.

Faces with ordinary material IDs use `CMAP` directly. Faces carrying flag
`0x2000` address the separate bitmap resource; until that texture container is
fully decoded, the exporter assigns the archive's neutral palette entry 48.

## Imported original models

| Archive | Name | Vertices | Faces | Runtime role |
|---|---:|---:|---:|---|
| SIM3D1.MAX | AP230 | 171 | 323 | City structure |
| SIM3D1.MAX | AR254 | 529 | 399 | City landmark |
| SIM3D1.MAX | CO182 | 556 | 316 | City complex |
| SIM3D1.MAX | ST215 | 181 | 144 | City structure |
| SIM3D2.MAX | BELL212 | 327 | 563 | Player helicopter body |
| SIM3D2.MAX | BELLROTR | 98 | 86 | Player main rotor |
| SIM3D3.MAX | IN191 | 216 | 212 | City structure |
| SIM3D3.MAX | PP204 | 320 | 437 | City structure |
| SIM3D3.MAX | PK238 | 516 | 289 | City structure |
| SIM3D3.MAX | ML239 | 436 | 231 | City structure |

`tools/extract_sim3d_models.py` performs the binary decode. The ignored
`Intermediate/RecoveredModels` directory is reproducible intermediate data.
`tools/import_original_models.py` imports it into `/Game/Original/Models`,
replaces temporary Interchange material instances with project-owned palette
materials, enables instanced-static-mesh shader usage, and saves collision as
complex-as-simple.
