# Outputs version 1 SPRC files
# These will automatically be migrated when loaded

# https://github.com/0xTiger/anvil-parser2
import anvil
import numpy as np
import math
import os

region = anvil.Region.from_file('r.0.0.mca')

SPIRE_CHUNK_SIZE = 64

class SpireChunk:
    def __init__(self, chunkCoords):
        self.chunkCoords = chunkCoords
        self.voxelData = np.array([np.uint32(0)] * (SPIRE_CHUNK_SIZE * SPIRE_CHUNK_SIZE * SPIRE_CHUNK_SIZE), dtype=np.uint32)
    
    def set_type(self, x: int, y: int, z: int, type: np.uint32):
        assert x >= 0 and x < SPIRE_CHUNK_SIZE
        assert y >= 0 and y < SPIRE_CHUNK_SIZE
        assert z >= 0 and z < SPIRE_CHUNK_SIZE
        self.voxelData[x*SPIRE_CHUNK_SIZE*SPIRE_CHUNK_SIZE+y*SPIRE_CHUNK_SIZE+z] = type
    
    def get_type(self, x, y, z) -> np.uint32:
        return self.voxelData[x*SPIRE_CHUNK_SIZE*SPIRE_CHUNK_SIZE + y*SPIRE_CHUNK_SIZE + z]

chunks = {}

def get_spire_chunk_coords(x, y, z):
    return (math.floor(x / float(SPIRE_CHUNK_SIZE)), math.floor(y / float(SPIRE_CHUNK_SIZE)), math.floor(z / float(SPIRE_CHUNK_SIZE)))

def get_spire_chunk(x, y, z) -> SpireChunk: 
    chunkCoords = get_spire_chunk_coords(x, y, z)
    if chunkCoords not in chunks:
        chunks[chunkCoords] = SpireChunk(chunkCoords)
    return chunks[chunkCoords]

def parse_minecraft_chunk(chunk, chunkOriginWorld):
    for x in range(16):
        for y in range(-64, 320):
            for z in range(16):
                spireChunk = get_spire_chunk(x + chunkOriginWorld[0], y + chunkOriginWorld[1], z + chunkOriginWorld[2])
                coordsInChunk = (x + chunkOriginWorld[0] - spireChunk.chunkCoords[0] * SPIRE_CHUNK_SIZE, y + chunkOriginWorld[1] - spireChunk.chunkCoords[1] * SPIRE_CHUNK_SIZE, z + chunkOriginWorld[2] - spireChunk.chunkCoords[2] * SPIRE_CHUNK_SIZE) 
                type = 2
                block = chunk.get_block(x, y, z)
                if block.id == "grass_block":
                    type = 1
                elif block.id == "air":
                    type = 0
                spireChunk.set_type(coordsInChunk[0], coordsInChunk[1], coordsInChunk[2], type)

for chunkX in range(32):
    for chunkY in range(32):
            print("Parsing Minecraft chunk " + str(chunkX) + ", " + str(chunkY) + ". (32x32 total chunks to parse)")
            chunk = anvil.Chunk.from_region(region, chunkX, chunkY)
            parse_minecraft_chunk(chunk, (chunkX * 16, 0, chunkY * 16))

print("Converted " + str(32*32) + " Minecraft chunks into " + str(len(chunks)) + " Spire chunks.")
OUTPUT_WORLD = "Test6"

remainingToWrite = len(chunks)

if not os.path.isdir(OUTPUT_WORLD):
    os.makedirs(OUTPUT_WORLD)
for chunk in chunks.values():
    print("Writing bytes of chunk " + str(chunk.chunkCoords[0]) + ", " + str(chunk.chunkCoords[1]) + ", " + str(chunk.chunkCoords[2]) + ". Remaining chunks to write: " + str(remainingToWrite))
    with open(OUTPUT_WORLD + "/" + str(chunk.chunkCoords[0]) + "_" + str(chunk.chunkCoords[1]) + "_" + str(chunk.chunkCoords[2]) +  ".sprc", mode="wb") as file:
        file.write(b"SPRVXLCHNK")
        file.write(np.uint32(1))
        for coord in chunk.chunkCoords:
            file.write(np.int32(coord))
        file.write(chunk.voxelData.tobytes())
        remainingToWrite -= 1
