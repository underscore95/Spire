#pragma once
#include "BufferAllocator.h"
#include "Chunk.h"
#include "Utils/Hashing.h"

namespace SpireVoxel {
    class VoxelWorld {
    public:
        struct WorldEditRequiredChanges {
            bool RecreatePipeline;
            bool RecreateOnlyCommandBuffers;
        };

    public:
        explicit VoxelWorld(Spire::RenderingManager &renderingManager);

        ~VoxelWorld();

    public:
        Chunk &LoadChunk(glm::ivec3 chunkPosition);

        void LoadChunks(const std::vector<glm::ivec3> &chunkPositions);

        void UnloadChunks(const std::vector<glm::ivec3> &chunkPositions);

        [[nodiscard]] Chunk *GetLoadedChunk(glm::ivec3 chunkPosition);

        DelegateSubscribers<WorldEditRequiredChanges> &GetOnWorldEditSubscribers();

        void CmdRender(VkCommandBuffer commandBuffer);

        void PushDescriptors(Spire::DescriptorSetLayout &constantDataLayout, Spire::DescriptorSetLayout &chunkVertexBuffersLayout);

        [[nodiscard]] std::size_t NumLoadedChunks() const;

        void UpdateChunkDatasBuffer();

        void OnChunkEdited(Chunk &chunk);

        void Update();

        std::unordered_map<glm::ivec3, Chunk>::iterator begin();

        std::unordered_map<glm::ivec3, Chunk>::iterator end();

        std::uint32_t GetNumLoadedChunks() const;

        void UnloadAllChunks();

    private:
        static constexpr glm::u32 CHUNK_VERTEX_BUFFER_SIZE = 1024 * 64; // 64kb

        Spire::RenderingManager &m_renderingManager;
        // https://en.cppreference.com/w/cpp/container/unordered_map.html - always iterates in the same order if the map hasnt been changed
        std::unordered_map<glm::ivec3, Chunk> m_chunks;
        Delegate<WorldEditRequiredChanges> m_onWorldEditedDelegate;
        Spire::VulkanBuffer m_chunkDatasBuffer;
        BufferAllocator m_chunkVertexBufferAllocator;
    };
} // SpireVoxel
