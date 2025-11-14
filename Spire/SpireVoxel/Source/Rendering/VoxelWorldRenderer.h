#pragma once

#include "Chunk/Chunk.h"

namespace SpireVoxel {
    class VoxelWorld;

    class VoxelWorldRenderer {
        friend class VoxelWorld;

    public:
        struct WorldEditRequiredChanges {
            bool RecreatePipeline;
            bool RecreateOnlyCommandBuffers;
        };

        explicit VoxelWorldRenderer(
            VoxelWorld &world,
            Spire::RenderingManager &renderingManager
        );

        ~VoxelWorldRenderer();

    public:
        void Update();

        DelegateSubscribers<WorldEditRequiredChanges> &GetOnWorldEditSubscribers();

        void CmdRender(VkCommandBuffer commandBuffer) const;

        void PushDescriptors(Spire::DescriptorSetLayout &constantDataLayout, Spire::DescriptorSetLayout &chunkVertexBuffersLayout);

        void UpdateChunkDatasBuffer() const;

        void NotifyChunkEdited(Chunk &chunk);

    private:
        void NotifyChunkLoadedOrUnloaded();

    private:
        static constexpr glm::u32 CHUNK_VERTEX_BUFFER_SIZE = 1024 * 64; // 64kb

        VoxelWorld &m_world;

        Spire::RenderingManager &m_renderingManager;
        Delegate<WorldEditRequiredChanges> m_onWorldEditedDelegate;
        Spire::VulkanBuffer m_chunkDatasBuffer;
        BufferAllocator m_chunkVertexBufferAllocator;
    };
} // SpireVoxel
