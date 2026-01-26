#pragma once

#include "VoxelRenderer.h"
#include "Chunk/Chunk.h"
#include "Chunk/Meshing/ChunkMesh.h"
#include "Chunk/Meshing/ChunkMesher.h"

namespace SpireVoxel {
    struct PushConstantsData;

    class VoxelWorld;

    // Handles rendering of a VoxelWorld
    class VoxelWorldRenderer {
        friend class VoxelWorld;

    public:
        explicit VoxelWorldRenderer(
            VoxelWorld &world,
            Spire::RenderingManager &renderingManager,
            const std::function<void()> &recreatePipelineCallback,
            bool isProfilingMeshing
        );

    public:
        // Call once per frame
        void Render(glm::u32 swapchainImageIndex);

        DelegateSubscribers<> &GetOnWorldEditSubscribers();

        // Record draw commands
        void CmdRender(VkCommandBuffer commandBuffer, glm::u32 swapchainImage, const Spire::Pipeline &pipeline) const;

        void PushDescriptors(Spire::PerImageDescriptorSetLayout &perFrameSet, Spire::DescriptorSetLayout &chunkVertexBuffersLayout);

        // Update chunk data buffer, needs to be called whenever a chunk is loaded or unloaded
        void UpdateChunkDatasBuffer();

        // Replicate edits to chunks to the GPU
        void NotifyChunkEdited(const Chunk &chunk);

        void HandleChunkEdits();

    private:
        void NotifyChunkLoadedOrUnloaded();

        void UpdateChunkDataCache();

        void FreeChunkVertexBuffer(Chunk &chunk);

        void FreeChunkVoxelDataBuffer(Chunk &chunk);

        [[nodiscard]] PushConstantsData CreatePushConstants() const;

    private:
        static constexpr glm::u32 MAXIMUM_VERTICES_IN_WORLD = 36 * (SPIRE_VOXEL_CHUNK_VOLUME * 48);
        static constexpr glm::u32 MAXIMUM_LOADED_CHUNKS = 384 * 3;

        VoxelWorld &m_world;

        Spire::RenderingManager &m_renderingManager;
        Delegate<> m_onWorldEditedDelegate;
        Spire::BufferAllocator m_chunkVertexBufferAllocator;
        // stores types of voxels
        Spire::BufferAllocator m_chunkVoxelDataBufferAllocator;
        // see ChunkData
        std::unique_ptr<Spire::PerImageBuffer> m_chunkDatasBuffer;
        // if {true,false,false} it means we need to update buffer 0 on swapchain image index 0
        // so next time frame % num swapchain images == 0, we'll upload the new data
        std::vector<bool> m_dirtyChunkDataBuffers;
        std::vector<ChunkData> m_latestCachedChunkData;
        std::unordered_set<glm::ivec3> m_editedChunks;
        std::unique_ptr<ChunkMesher> m_chunkMesher;
    };
} // SpireVoxel
