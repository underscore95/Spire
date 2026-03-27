# converts spire world from v2 to v1

import struct
import os
import sys

IDENTIFIER = b"SPRVXLCHNK"
SIZE = 64 * 64 * 64

INPUT_DIR = "Test3"
OUTPUT_DIR = INPUT_DIR + "-v1"

if os.path.exists(OUTPUT_DIR):
    raise RuntimeError(f"Output directory already exists: {OUTPUT_DIR}")

os.makedirs(OUTPUT_DIR)

for root, _, files in os.walk(INPUT_DIR):
    rel_path = os.path.relpath(root, INPUT_DIR)
    out_root = os.path.join(OUTPUT_DIR, rel_path) if rel_path != "." else OUTPUT_DIR
    os.makedirs(out_root, exist_ok=True)

    for file in files:
        if not file.endswith(".sprc"):
            continue

        in_path = os.path.join(root, file)
        out_path = os.path.join(out_root, file)

        with open(in_path, "rb") as f:
            identifier = f.read(len(IDENTIFIER))
            if identifier != IDENTIFIER:
                raise ValueError(f"{in_path}: invalid identifier")

            version, = struct.unpack("<I", f.read(4))
            if version != 2:
                raise ValueError(f"{in_path}: not version 2")

            chunk_x, chunk_y, chunk_z = struct.unpack("<iii", f.read(12))

            voxel_data = struct.unpack(f"<{SIZE}H", f.read(SIZE * 2))

        voxel_data_i32 = [int(v) for v in voxel_data]

        with open(out_path, "wb") as f:
            f.write(IDENTIFIER)
            f.write(struct.pack("<I", 1))
            f.write(struct.pack("<iii", chunk_x, chunk_y, chunk_z))
            f.write(struct.pack(f"<{SIZE}i", *voxel_data_i32))