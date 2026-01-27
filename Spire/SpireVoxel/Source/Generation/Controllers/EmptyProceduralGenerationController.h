#pragma once
#include "IProceduralGenerationController.h"

// Never generates chunks
namespace SpireVoxel {
    class EmptyProceduralGenerationController : public IProceduralGenerationController {
    public:
        std::vector<glm::ivec3> GetChunkCoordsToLoad(VoxelWorld &world, const IVoxelCamera &camera, glm::u32 maxToLoad) override { return {}; }
    };
} // SpireVoxel
