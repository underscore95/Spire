#pragma once

#include "Chunk/Chunk.h"
#include "ChunkOrderControllers/IChunkOrderController.h"

namespace SpireVoxel {
    class SimpleLODController : public IChunkOrderController {
    public:
        explicit SimpleLODController(VoxelWorld &world);

    public:
        [[nodiscard]] std::vector<glm::ivec3> GetChunkCoordsBatch(VoxelWorld &world, const IVoxelCamera &camera, glm::u32 maxBatchSize) override;

    protected:
        virtual glm::u32 GetLODLevel(float chunkDistance);

    private:

        Chunk *FindFurthestLOD1Chunk(glm::ivec3 cameraChunk, std::vector<glm::ivec3> &ignore) const;

    private:
        VoxelWorld &m_world;
    };
} // SpireVoxel
