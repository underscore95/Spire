#pragma once

#include "EngineIncludes.h"
#include "ISamplingOffsets.h"
#include "Chunk/Chunk.h"

namespace SpireVoxel {
    class VoxelWorld;

    class LODManager {
        friend class VoxelWorld;

    private:
        LODManager(VoxelWorld &world, std::shared_ptr<ISamplingOffsets> samplingOffsets);

    public:
        void IncreaseLODTo(Chunk &chunk, glm::u32 newLODScale);

    private:
        VoxelWorld &m_world;
        std::shared_ptr<ISamplingOffsets> m_samplingOffsets;
    };
} // SpireVoxel
