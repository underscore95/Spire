#pragma once
#include "IChunkOrderController.h"

// Never generates chunks
namespace SpireVoxel {
    class EmptyChunkOrderController : public IChunkOrderController {
    public:
        std::vector<glm::ivec3> GetChunkCoordsBatch(VoxelWorld &world, const IVoxelCamera &camera, glm::u32 maxToLoad) override { return {}; }
    };
} // SpireVoxel
