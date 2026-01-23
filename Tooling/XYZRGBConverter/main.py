# Outputs version 2 SPRC files
# You can use https://github.com/eisenwave/obj2voxel to convert a .obj to a .xyzrgb
# This script will then convert a .xyzrgb into a folder of .sprc which Spire can render

import os
import struct
import sys
from collections import defaultdict


CHUNK_SIZE = 64
VOXELS_PER_CHUNK = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE
HEADER_ID = b"SPRVXLCHNK"
FORMAT_VERSION = 2

def voxel_index(x, y, z):
    return x + y * CHUNK_SIZE + z * CHUNK_SIZE * CHUNK_SIZE

def parse_xyzrgb(path):
    voxels = []
    with open(path, "r") as f:
        for line in f:
            if not line.strip():
                continue
            x, y, z, *_ = line.split()
            voxels.append((int(x), int(y), int(z)))
    return voxels

def chunk_coords(z, y, x):
    return (
        x // CHUNK_SIZE,
        y // CHUNK_SIZE,
        z // CHUNK_SIZE
    )


def local_coords(x, y, z):
    return (
        x % CHUNK_SIZE,
        y % CHUNK_SIZE,
        z % CHUNK_SIZE
    )


def write_sprc(path, cx, cy, cz, voxel_data):
    with open(path, "wb") as f:
        f.write(HEADER_ID)
        f.write(struct.pack("<I", FORMAT_VERSION))
        f.write(struct.pack("<iii", cx, cy, cz))
        f.write(struct.pack("<" + "H" * VOXELS_PER_CHUNK, *voxel_data))

def convert_xyzrgb_to_sprc(xyzrgb_path, output_folder):
    os.makedirs(output_folder, exist_ok=True)

    points = parse_xyzrgb(xyzrgb_path)

    chunks = defaultdict(lambda: [0] * VOXELS_PER_CHUNK)

    for x, y, z in points:
        cx, cy, cz = chunk_coords(x, y, z)
        lx, ly, lz = local_coords(x, y, z)
        idx = voxel_index(lx, ly, lz)
        chunks[(cx, cy, cz)][idx] = 1

    for (cx, cy, cz), voxel_data in chunks.items():
        filename = f"chunk_{cx}_{cy}_{cz}.sprc"
        path = os.path.join(output_folder, filename)
        write_sprc(path, cx, cy, cz, voxel_data)

convert_xyzrgb_to_sprc("spider.xyzrgb", "Test7")
