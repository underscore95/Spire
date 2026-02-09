#pragma once

#include "EngineIncludes.h"

namespace SpireVoxel {
    struct Chunk;

    struct DetailLevel {
        // 1 = max detail
        // 2 = chunk is double size
        // 3 = chunk is triple size
        glm::u32 Scale = 1;

        // Check if a chunk contains the chunkCoords when scaled using its LOD
        // Note this will always return true if possibleLODChunk.ChunkPosition == chunkCoords
        [[nodiscard]] static bool ChunkIncludes(const Chunk& possibleLODChunk, glm::ivec3 chunkCoords);
    };
} // SpireVoxel
