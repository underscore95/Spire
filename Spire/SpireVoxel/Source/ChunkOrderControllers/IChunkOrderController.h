#pragma once

#include "EngineIncludes.h"

namespace SpireVoxel {
    class IVoxelCamera;
    class VoxelWorld;
}

namespace SpireVoxel {
    // A controller decides what chunks should have an operation applied
    class IChunkOrderController {
    public:
        virtual ~IChunkOrderController() = default;

    public:
        [[nodiscard]] virtual std::vector<glm::ivec3> GetChunkCoordsToLoad(VoxelWorld &world, const IVoxelCamera &camera, glm::u32 maxToLoad) = 0;

        [[nodiscard]] static glm::ivec3 GetSpiral(glm::u32 index);
    };
} // SpireVoxel
