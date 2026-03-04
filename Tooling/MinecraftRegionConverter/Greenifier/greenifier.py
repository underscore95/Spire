import sys
import shutil
import subprocess
from pathlib import Path

ADD_R = -40
ADD_G = 30
ADD_B = -80

FILES = [
    "oak_leaves.png",
    "birch_leaves.png",
    "spruce_leaves.png",
    "dark_oak_leaves.png",
    "jungle_leaves.png",
    "mangrove_leaves.png",
    "pale_oak_leaves.png",
    "acacia_leaves.png",
    "grass_block_top.png",
]

if len(sys.argv) != 3:
    print("Usage: python script.py <input_dir> <output_dir>")
    sys.exit(1)

input_dir = Path(sys.argv[1])
output_dir = Path(sys.argv[2])

if not input_dir.is_dir():
    print("Input directory does not exist or is not a directory")
    sys.exit(1)

if output_dir.exists():
    print("Output directory already exists")
    sys.exit(1)

shutil.copytree(input_dir, output_dir)

vf = (
    f"lutrgb="
    f"r='clip(val+{ADD_R},0,255)':"
    f"g='clip(val+{ADD_G},0,255)':"
    f"b='clip(val+{ADD_B},0,255)'"
)

for name in FILES:
    src = input_dir / name
    dst = output_dir / name

    if not src.is_file():
        print(f"{src} not found")
        continue

    if dst.exists():
        dst.unlink()

    result = subprocess.run([
        "ffmpeg",
        "-i", str(src),
        "-vf", vf,
        str(dst)
    ])

    if result.returncode != 0:
        print(f"ffmpeg failed for {name}")