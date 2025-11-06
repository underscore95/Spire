#pragma once
#include "Chunk.h"
#include "Utils/Hashing.h"

namespace SpireVoxel {
    class VoxelWorld {
    public:
        VoxelWorld(Spire::RenderingManager& renderingManager);

    public:
        Chunk& LoadChunk(glm::ivec3 chunkPosition);
        void LoadChunks(const std::vector<glm::ivec3>& chunkPositions);

        void UnloadChunk(glm::ivec3 chunkPosition);

        DelegateSubscribers<> &GetOnChunkLoadOrUnloadSubscribers();

        void CmdRender(VkCommandBuffer commandBuffer);

        void PushDescriptors(Spire::DescriptorSetLayout &layout, glm::u32 binding);

        std::size_t NumLoadedChunks() const;

    private:
        Spire::RenderingManager& m_renderingManager;
        std::unordered_map<glm::ivec3, Chunk> m_chunks;
        Delegate<> m_onChunkLoadOrUnloadDelegate;
    };
} // SpireVoxel
