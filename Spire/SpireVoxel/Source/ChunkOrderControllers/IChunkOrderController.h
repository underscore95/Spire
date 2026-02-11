#pragma once

#include "EngineIncludes.h"

namespace SpireVoxel {
    class IVoxelCamera;
    class VoxelWorld;
}

namespace SpireVoxel {
    // A controller decides what chunks should have an operation applied
    // depending on usage, this could be procedural generation, converting to a higher LOD ...
    class IChunkOrderController {
    public:
        virtual ~IChunkOrderController() = default;

    public:
        [[nodiscard]] virtual std::vector<glm::ivec3> GetChunkCoordsBatch(VoxelWorld &world, const IVoxelCamera &camera, glm::u32 maxBatchSize) = 0;

        [[nodiscard]] static glm::ivec3 GetSpiral(glm::u32 index);
    };
} // SpireVoxel
