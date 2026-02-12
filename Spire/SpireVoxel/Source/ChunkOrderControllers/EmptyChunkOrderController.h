#pragma once
#include "IChunkOrderController.h"

// Never generates chunks
namespace SpireVoxel {
    class EmptyChunkOrderController : public IChunkOrderController {
    public:
        std::vector<glm::ivec3> GetChunkCoordsToLoad(VoxelWorld &world, const IVoxelCamera &camera, glm::u32 maxToLoad) override { return {}; }
    };
} // SpireVoxel
