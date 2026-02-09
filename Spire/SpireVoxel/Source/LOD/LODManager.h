#pragma once

#include "EngineIncludes.h"
#include "ISamplingOffsets.h"
#include "Chunk/Chunk.h"

namespace SpireVoxel {
    class VoxelWorld;

    class LODManager {
        friend class VoxelWorld;

    private:
        LODManager(VoxelWorld &world, const std::shared_ptr<ISamplingOffsets> &samplingOffsets);

    private:
        void OnChunkUnload(const Chunk& chunk);

    public:
        void IncreaseLODTo(Chunk &chunk, glm::u32 newLODScale);

        // Get a chunk if it is loaded
        // If the chunk is covered by a LOD chunk, get that instead
        [[nodiscard]] Chunk* TryGetLODChunk(glm::ivec3 chunkCoords);

    private:
        VoxelWorld &m_world;
        std::shared_ptr<ISamplingOffsets> m_samplingOffsets;
        std::unordered_map<glm::ivec3, Chunk* > m_coveredToPrimaryChunk;
    };
} // SpireVoxel
