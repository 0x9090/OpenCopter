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

The face-material binder at `0x004721a0` consumes the FACE flags and material
field. The recovered Bell body uses palette/shading face types (`0x0003` and
`0x4002`), not bitmap-mapped faces. Its visible materials therefore come from
`CMAP`; an absent `SIM3D.BMP` binding is not the cause of a black Bell. The
runtime importer creates rough base-color materials, converts the palette's
sRGB bytes to linear color, and rebinds every mesh after Interchange cleanup so stale deleted
material-instance references cannot turn the models black.

`SIM3D1.MAX` supplies the master palette used by the game. Entries other than
zero agree across all three archives; later archives store archive-specific
key colors in entry zero. Retaining those later zero values makes the Bell's
neutral shell magenta, so the exporter uses SIM3D1's entry zero for every
archive.

The `BELLROTR` spinning-disc interior uses material 6 on `0x0002` faces. Its
bright green palette value is a transparency key, not an opaque rotor color;
the Unreal material discards it while retaining the separately modeled rotor
rim.

## SIM3D.BMP texture container

`SIM3D.BMP` is a proprietary indexed-bitmap archive, not a Windows bitmap.
`InitializeGameResources` (`0x0047de90`) loads it with resource type 3 and
`0x0046f4b0` decodes its 68 entries. The file starts with four 32-bit values
(`file size`, zero, entry count, entry count), followed by 68 twelve-byte
directory records. Each sequential image record contains:

- width, height, and a reserved 32-bit value;
- one 32-bit row offset per scanline;
- width x height indexed pixels.

The record length is `12 + (width + 4) * height`. This decode is retained for
future texture-mapped models, but Ghidra and the FACE records confirm the Bell
212 itself is palette-shaded.

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
