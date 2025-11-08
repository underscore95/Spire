#pragma once
#include "Chunk.h"
#include "Utils/Hashing.h"

namespace SpireVoxel {
    class VoxelWorld {
    public:
        explicit VoxelWorld(Spire::RenderingManager &renderingManager);

        ~VoxelWorld();

    public:
        Chunk &LoadChunk(glm::ivec3 chunkPosition);

        void LoadChunks(const std::vector<glm::ivec3> &chunkPositions);

        void UnloadChunk(glm::ivec3 chunkPosition);

        DelegateSubscribers<> &GetOnPipelineRecreationRequiredSubscribers();

        void CmdRender(VkCommandBuffer commandBuffer);

        void PushDescriptors(Spire::DescriptorSetLayout &constantDataLayout, Spire::DescriptorSetLayout &chunkVertexBuffersLayout);

        [[nodiscard]] std::size_t NumLoadedChunks() const;

        void CreateChunkDatasBuffer();
    private:
        void FreeChunkDatasBuffer();

    private:
        Spire::RenderingManager &m_renderingManager;
        std::unordered_map<glm::ivec3, Chunk> m_chunks; // https://en.cppreference.com/w/cpp/container/unordered_map.html - always iterates in the same order if the map hasnt been changed
        Delegate<> m_onPipelineRecreationRequiredDelegate;
        Spire::VulkanBuffer m_chunkDatasBuffer;
    };
} // SpireVoxel
