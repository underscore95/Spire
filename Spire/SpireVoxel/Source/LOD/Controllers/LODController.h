#pragma once

#include "EngineIncludes.h"
#include "Chunk/VoxelWorld.h"

namespace SpireVoxel {

class LODController {
public:
    struct LODChunk {
        glm::u32 LOD;
        glm::ivec3 ChunkCoords;
    };
public:


protected:
    // Returns the highest LOD level where the LOD won't cover another chunk with >1 LOD
    // For negative coordinates, we'll actually LOD a chunk far away enough that it barely covers the request chunk
    LODChunk GetBestLOD(VoxelWorld &world, glm::ivec3 coords, glm::u32 desiredLODLevel);
};

} // SpireVoxel