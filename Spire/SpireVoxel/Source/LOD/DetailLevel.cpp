#include "DetailLevel.h"

#include "Chunk/Chunk.h"

bool SpireVoxel::DetailLevel::ChunkIncludes(const Chunk &possibleLODChunk, glm::ivec3 chunkCoords) {
    assert(possibleLODChunk.LOD.Scale >= 1);

    // If LOD chunk is > in any axis, it cannot contain the chunk
    if (possibleLODChunk.ChunkPosition.x > chunkCoords.x || possibleLODChunk.ChunkPosition.y > chunkCoords.y || possibleLODChunk.ChunkPosition.z > chunkCoords.z) return false;
    glm::ivec3 maxLODChunkCoords = possibleLODChunk.ChunkPosition + glm::ivec3{1, 1, 1} * static_cast<int>(possibleLODChunk.LOD.Scale - 1);
    // After LOD scaling, LOD chunk isn't big enough to contain ours
    if (maxLODChunkCoords.x > chunkCoords.x || maxLODChunkCoords.y > chunkCoords.y || maxLODChunkCoords.z > chunkCoords.z) return false;
    return true;
}
