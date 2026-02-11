#pragma once
#include "../../ChunkOrderControllers/IChunkOrderController.h"

namespace SpireVoxel {
    // Generates chunks in a spiral pattern around the camera, only generates chunks in yRange (inclusive both ends)
    class SimpleProceduralGenerationController : public IChunkOrderController {
    public:
        // view distance in chunks
        explicit SimpleProceduralGenerationController(glm::u32 viewDistance, glm::ivec2 yRange);

    public:
        std::vector<glm::ivec3> GetChunkCoordsBatch(VoxelWorld &world, const IVoxelCamera &camera, glm::u32 maxToLoad) override;

    private:
        glm::ivec3 GetNextUnloadedChunk(glm::u32 &spiralIndex, VoxelWorld &world, glm::ivec3 cameraChunkPos, const std::vector<glm::ivec3> &alreadyFound) const;

    private:
        glm::u32 m_viewDistance;
        glm::ivec2 m_yRange;
    };
} // SpireVoxel
