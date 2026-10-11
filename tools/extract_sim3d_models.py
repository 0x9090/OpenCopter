#!/usr/bin/env python3
"""Extract SimCopter's original GEOM/OBJX records to Wavefront OBJ files.

The format was recovered from the SimCopter 1.0 executable's loader at
0x0046ec40 and object decoder at 0x00470c00.  SIM3D*.MAX is not an Autodesk
3ds Max file: it is a Maxis DIRC archive containing a CMAP palette and a GEOM
directory.  Geometry uses signed 16.16 fixed-point coordinates and FACE
records with one-based vertex indices.
"""

from __future__ import annotations

import argparse
import json
import struct
from dataclasses import dataclass
from pathlib import Path


FIXED_SCALE = 65536.0
# The recovered coordinate system uses Y-up.  Forty Unreal centimeters per
# source unit puts the 44-unit Bell 212 at its real-world ~17.7 m length.
UNREAL_CENTIMETERS_PER_UNIT = 40.0


@dataclass(frozen=True)
class DirectoryEntry:
    name: str
    offset: int
    part_count: int
    counts: tuple[int, int, int, int, int, int]


@dataclass(frozen=True)
class Face:
    indices: tuple[int, ...]
    uvs: tuple[tuple[float, float], ...]
    flags: int
    material_id: int
    shade: int
    detail: int


@dataclass(frozen=True)
class Model:
    name: str
    vertices: tuple[tuple[float, float, float], ...]
    faces: tuple[Face, ...]


class Sim3DArchive:
    def __init__(self, path: Path) -> None:
        self.path = path
        self.data = path.read_bytes()
        self.palette = self._read_palette()
        self.entries = self._read_directory()

    def _read_palette(self) -> tuple[tuple[int, int, int], ...]:
        if self.data[:4] != b"DIRC":
            raise ValueError(f"{self.path} is not a SimCopter DIRC archive")
        entry_count = struct.unpack_from("<I", self.data, 8)[0]
        chunks = {
            tag: offset
            for tag, offset in (
                struct.unpack_from("<4sI", self.data, 12 + index * 8)
                for index in range(entry_count)
            )
        }
        cmap_offset = chunks[b"CMAP"]
        if self.data[cmap_offset : cmap_offset + 4] != b"CMAP":
            raise ValueError("CMAP directory offset does not point at CMAP data")
        # CMAP + size + version + NUL-terminated four-character name.
        palette_offset = cmap_offset + 17
        palette_bytes = self.data[palette_offset : palette_offset + 256 * 3]
        return tuple(
            tuple(palette_bytes[index : index + 3])
            for index in range(0, len(palette_bytes), 3)
        )

    def _read_directory(self) -> dict[str, DirectoryEntry]:
        entry_count = struct.unpack_from("<I", self.data, 8)[0]
        chunks = {
            tag: offset
            for tag, offset in (
                struct.unpack_from("<4sI", self.data, 12 + index * 8)
                for index in range(entry_count)
            )
        }
        geom_offset = chunks[b"GEOM"]
        if self.data[geom_offset : geom_offset + 4] != b"GEOM":
            raise ValueError("GEOM directory offset does not point at GEOM data")
        named_count = struct.unpack_from("<I", self.data, geom_offset + 8)[0]
        named_offset = struct.unpack_from("<I", self.data, geom_offset + 16)[0]

        entries: dict[str, DirectoryEntry] = {}
        for index in range(named_count):
            record = self.data[named_offset + index * 0x35 : named_offset + (index + 1) * 0x35]
            name = record[:17].split(b"\0", 1)[0].decode("ascii")
            data_offset, part_count = struct.unpack_from("<II", record, 17)
            counts = struct.unpack_from("<6I", record, 25)
            entries[name] = DirectoryEntry(name, data_offset, part_count, counts)
        return entries

    def read_model(self, name: str) -> Model:
        entry = self.entries[name]
        if entry.part_count != 1:
            raise ValueError(f"{name} is an aggregate with {entry.part_count} parts")

        offset = entry.offset
        if self.data[offset : offset + 4] != b"OBJX":
            raise ValueError(f"{name} does not point at an OBJX record")

        vertex_count = struct.unpack_from("<H", self.data, offset + 8)[0] - 1
        face_count = struct.unpack_from("<H", self.data, offset + 10)[0]
        source_name = self.data[offset + 20 : offset + 37].split(b"\0", 1)[0].decode("ascii")
        if source_name and source_name.casefold() != name.casefold():
            raise ValueError(f"Directory name {name} disagrees with OBJX name {source_name}")

        cursor = offset + 124
        # Object translation/pivot is stored separately.  Source vertices are
        # already in the common model coordinate system, which is required for
        # the independently animated rotor meshes to align with their bodies.
        cursor += 12
        vertices = []
        for _ in range(vertex_count):
            source_x, source_y, source_z = struct.unpack_from("<iii", self.data, cursor)
            cursor += 12
            # Maxis: X=right, Y=up, Z=forward. Unreal: X=forward, Y=right, Z=up.
            vertices.append(
                (
                    source_z / FIXED_SCALE * UNREAL_CENTIMETERS_PER_UNIT,
                    source_x / FIXED_SCALE * UNREAL_CENTIMETERS_PER_UNIT,
                    source_y / FIXED_SCALE * UNREAL_CENTIMETERS_PER_UNIT,
                )
            )

        faces = []
        for _ in range(face_count):
            if self.data[cursor : cursor + 4] != b"FACE":
                raise ValueError(f"Malformed FACE record in {name} at 0x{cursor:x}")
            record_size = struct.unpack_from("<I", self.data, cursor + 4)[0]
            corner_count = struct.unpack_from("<H", self.data, cursor + 8)[0]
            flags, material_id = struct.unpack_from("<II", self.data, cursor + 10)
            shade = self.data[cursor + 18]
            detail = struct.unpack_from("<H", self.data, cursor + 19)[0]
            indices = tuple(
                value - 1
                for value in struct.unpack_from(
                    f"<{corner_count}H", self.data, cursor + 21
                )
            )
            uv_offset = cursor + 21 + corner_count * 2
            uvs = tuple(
                (
                    struct.unpack_from("<i", self.data, uv_offset + corner * 8)[0] / FIXED_SCALE,
                    1.0
                    - struct.unpack_from("<i", self.data, uv_offset + corner * 8 + 4)[0]
                    / FIXED_SCALE,
                )
                for corner in range(corner_count)
            )
            expected_size = 21 + corner_count * 10
            if record_size != expected_size:
                raise ValueError(
                    f"FACE size mismatch in {name}: {record_size} != {expected_size}"
                )
            if any(index < 0 or index >= vertex_count for index in indices):
                raise ValueError(f"FACE in {name} contains an out-of-range vertex index")
            faces.append(Face(indices, uvs, flags, material_id, shade, detail))
            cursor += record_size

        return Model(name, tuple(vertices), tuple(faces))

    def face_palette_index(self, face: Face) -> int:
        # 0x2000 marks texture-mapped faces whose material IDs address the
        # separate SIM3D.BMP resource. Until that bitmap container is decoded,
        # use the archive's neutral facade color. All ordinary colored faces
        # address CMAP directly.
        if face.flags & 0x2000 or face.material_id > 255:
            return 48
        return face.material_id

    def export_obj(self, model: Model, output_dir: Path) -> dict[str, object]:
        output_dir.mkdir(parents=True, exist_ok=True)
        obj_path = output_dir / f"{model.name}.obj"
        mtl_path = output_dir / f"{model.name}.mtl"
        palette_indices = sorted({self.face_palette_index(face) for face in model.faces})

        mtl_lines = []
        for palette_index in palette_indices:
            red, green, blue = self.palette[palette_index]
            mtl_lines.extend(
                [
                    f"newmtl {model.name}_Palette_{palette_index:03d}",
                    f"Kd {red / 255.0:.6f} {green / 255.0:.6f} {blue / 255.0:.6f}",
                    "Ka 0.000000 0.000000 0.000000",
                    "Ks 0.080000 0.080000 0.080000",
                    "Ns 12.000000",
                    "illum 2",
                    "",
                ]
            )
        mtl_path.write_text("\n".join(mtl_lines), encoding="ascii")

        obj_lines = [f"mtllib {mtl_path.name}", f"o {model.name}"]
        obj_lines.extend(f"v {x:.6f} {y:.6f} {z:.6f}" for x, y, z in model.vertices)

        uv_index = 1
        for face in model.faces:
            for u, v in face.uvs:
                obj_lines.append(f"vt {u:.6f} {v:.6f}")
        current_material = None
        for face in model.faces:
            # The original renderer also stores line/point primitives in FACE
            # records. Wavefront/Unreal static meshes require polygons, so keep
            # their vertices but omit non-surface primitives from the OBJ.
            if len(face.indices) < 3 or len(set(face.indices)) < 3:
                uv_index += len(face.indices)
                continue
            palette_index = self.face_palette_index(face)
            if palette_index != current_material:
                obj_lines.append(f"usemtl {model.name}_Palette_{palette_index:03d}")
                current_material = palette_index
            corners = []
            for vertex_index in face.indices:
                corners.append(f"{vertex_index + 1}/{uv_index}")
                uv_index += 1
            obj_lines.append("f " + " ".join(corners))
        obj_path.write_text("\n".join(obj_lines) + "\n", encoding="ascii")

        mins = tuple(min(vertex[axis] for vertex in model.vertices) for axis in range(3))
        maxs = tuple(max(vertex[axis] for vertex in model.vertices) for axis in range(3))
        return {
            "archive": self.path.name,
            "name": model.name,
            "vertices": len(model.vertices),
            "faces": len(model.faces),
            "palette": {
                f"{palette_index:03d}": list(self.palette[palette_index])
                for palette_index in palette_indices
            },
            "bounds_cm": {"min": mins, "max": maxs},
            "obj": str(obj_path),
        }


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("archives", nargs="+", type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument(
        "--models",
        nargs="*",
        default=[],
        help="Named models to extract; omit to extract every non-aggregate model",
    )
    args = parser.parse_args()

    requested = {name.upper() for name in args.models}
    manifest = []
    found = set()
    master_palette = None
    for archive_path in args.archives:
        archive = Sim3DArchive(archive_path)
        # SIM3D1 carries the master game palette. Later archives have an
        # archive-specific color-key value in entry zero; using it as a surface
        # color turns the Bell 212 nose magenta instead of the original grey.
        if master_palette is None:
            master_palette = archive.palette
        else:
            archive.palette = master_palette
        names = [
            name
            for name, entry in archive.entries.items()
            if entry.part_count == 1 and (not requested or name.upper() in requested)
        ]
        for name in names:
            manifest.append(archive.export_obj(archive.read_model(name), args.output))
            found.add(name.upper())

    missing = requested - found
    if missing:
        raise SystemExit("Models not found: " + ", ".join(sorted(missing)))
    manifest_path = args.output / "manifest.json"
    manifest_path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    print(f"Extracted {len(manifest)} original models to {args.output}")


if __name__ == "__main__":
    main()
