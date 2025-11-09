#pragma once
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

        DelegateSubscribers<WorldEditRequiredChanges> &GetOnWorldEditSubscribers();

        void CmdRender(VkCommandBuffer commandBuffer);

        void PushDescriptors(Spire::DescriptorSetLayout &constantDataLayout, Spire::DescriptorSetLayout &chunkVertexBuffersLayout);

        [[nodiscard]] std::size_t NumLoadedChunks() const;

        void CreateOrUpdateChunkDatasBuffer();

        void OnChunkEdited(Chunk &chunk, glm::u32 oldVertexCount);

    private:
        void FreeChunkDatasBuffer();

    private:
        Spire::RenderingManager &m_renderingManager;
        std::unordered_map<glm::ivec3, Chunk> m_chunks;
        // https://en.cppreference.com/w/cpp/container/unordered_map.html - always iterates in the same order if the map hasnt been changed
        Delegate<WorldEditRequiredChanges> m_onWorldEditedDelegate;
        Spire::VulkanBuffer m_chunkDatasBuffer; // todo one per frame?
        Spire::VulkanBuffer m_dummyVertexBuffer; // used when no loaded chunks have a mesh so we can still push a descriptor for vertex buffer
    };
} // SpireVoxel
