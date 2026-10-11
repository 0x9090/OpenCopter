# Original-to-Unreal visual comparison

The reconstructed screens were compared at the original game's 640x480 design
resolution against the supplied assets and contemporary SimCopter captures.

## Main menu

The original composes `menusky.smk` as the full-screen background, then draws
`main1.bmp` at (0, 29), `main2.bmp` at (286, 0), and `main3.bmp` at (427, 315).
The Unreal HUD now uses the same coordinates and aspect-preserving scale, with
the original labels: New Career Game, Open Career Game, New User Game, Open
User Game, and Quit. The current build uses the first recovered frame of
`menusky.smk`; SMK animation playback remains future work.

## Flight HUD

The original gameplay capture places the recovered artwork as follows:

| Source artwork | Original 640x480 placement |
|---|---:|
| `dash4.bmp` | (0, 355), 455x43 |
| `dash2.bmp` | (556, 41), 84x291 |
| `dash5.bmp` | (455, 332), 185x148 |
| `dash6.bmp` | (0, 398), 458x82 |
| compass strip | (384, 365), 160x16 |

The Unreal canvas now uses those positions instead of the earlier rearranged
overlay. Dynamic speed, altitude, compass, mission, and distance text is drawn
inside the source-art display areas.

## World and aircraft

The player body and rotor are the `BELL212` and `BELLROTR` OBJX records from
`SIM3D2.MAX`; the city set is sourced from the corresponding SIM3D1/SIM3D3
records. Live PIE comparison confirmed that each saved mesh now points to the
persistent `M_SIM3D*_Palette_*` materials rather than deleted Interchange
instances. The materials render with rough, directionally lit recovered
palette base colors, and the ground uses the original `grndhitx.bmp` surface. The chase camera is placed
behind the full 17.7 m aircraft rather than inside its tail boom.
The Bell rotor's palette-6 green key is transparent, preventing the keyed
spinning-disc interior from appearing as an opaque fluorescent polygon.
