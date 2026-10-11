# SimCopter reverse-engineering workspace

This directory keeps the original disc extraction and analysis outputs separate
from the Unreal Engine reconstruction.

## Inputs

- `../SimCopter.nrg`: original Nero disc image supplied for this project.
- `disc/`: files extracted from the ISO-9660 session with 7-Zip.
- `disc/SIMCOPTER/SimCopter.exe`: original 32-bit Windows game executable.

## Tooling

Run `../tools/start-ghidra-mcp.sh` to start the local Ghidra 12.1.4 headless
server with GhidraMCP 7.0.0 on `127.0.0.1:8089`. The server uses the
workspace-local `.ghidra-profile` directory and does not change the shared
toolchain installation.

Analysis notes and generated inventories belong in `reports/`. The rebuild
itself belongs in the top-level Unreal project, not in this directory.
