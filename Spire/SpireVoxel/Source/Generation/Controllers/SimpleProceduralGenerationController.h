#pragma once
#include "IProceduralGenerationController.h"

namespace SpireVoxel {
    // Generates chunks in a spiral pattern around the camera, only generates chunks at Y=0
    class SimpleProceduralGenerationController : public IProceduralGenerationController {
    public:
        std::vector<glm::ivec3> GetChunkCoordsToLoad(VoxelWorld &world, const IVoxelCamera &camera, glm::u32 maxToLoad) override;
    };
} // SpireVoxel
