#pragma once

#include "VoxelRenderer.h"
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

    public:
        void Render(glm::u32 swapchainImageIndex);

        DelegateSubscribers<WorldEditRequiredChanges> &GetOnWorldEditSubscribers();

        void CmdRender(glm::u32 swapchainImage, VkCommandBuffer commandBuffer) const;

        void PushDescriptors(Spire::PerImageDescriptorSetLayout &perFrameSet, Spire::DescriptorSetLayout &chunkVertexBuffersLayout);

        void UpdateChunkDatasBuffer();

        void NotifyChunkEdited(Chunk &chunk);

        void HandleChunkEdits();

    private:
        void NotifyChunkLoadedOrUnloaded();

        void UpdateChunkDataCache();

        void FreeChunkVertexBuffer(Chunk &chunk);

    private:
        static constexpr glm::u32 MAXIMUM_VERTICES_IN_WORLD = 36 * (SPIRE_VOXEL_CHUNK_VOLUME * 48);
        static constexpr glm::u32 MAXIMUM_LOADED_CHUNKS = 384 * 3;
        static constexpr glm::u32 MAXIMUM_RENDERED_VOXELS = MAXIMUM_VERTICES_IN_WORLD / 8; // maximum amount of voxels where at least 1 face is rendered

        VoxelWorld &m_world;

        Spire::RenderingManager &m_renderingManager;
        Delegate<WorldEditRequiredChanges> m_onWorldEditedDelegate;
        BufferAllocator m_chunkVertexBufferAllocator;
        std::unique_ptr<Spire::PerImageBuffer> m_chunkDatasBuffer;
        std::vector<bool> m_dirtyChunkDataBuffers; // if {true,false,false} it means we need to update buffer 0 on swapchain image index 0
        std::vector<ChunkData> m_latestCachedChunkData;
        std::unordered_set<glm::ivec3> m_editedChunks;
    };
} // SpireVoxel
