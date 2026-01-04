#pragma once

#include "VoxelRenderer.h"
#include "Chunk/Chunk.h"
#include "Chunk/Meshing/ChunkMesherManager.h"

namespace SpireVoxel {
    class VoxelWorld;

    class VoxelWorldRenderer {
        friend class VoxelWorld;

    public:
        struct WorldEditRequiredChanges {
            bool RecreatePipeline;
            bool RecreateOnlyCommandBuffers;

            WorldEditRequiredChanges &operator|=(const WorldEditRequiredChanges &other) {
                RecreatePipeline |= other.RecreatePipeline;
                RecreateOnlyCommandBuffers |= other.RecreateOnlyCommandBuffers;
                return *this;
            }

            [[nodiscard]] bool IsAnyChanges() const {
                return RecreatePipeline | RecreatePipeline;
            }
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

        WorldEditRequiredChanges UpdateMesh(Chunk &chunk, const std::vector<VertexData> &mesh);

    private:
        void NotifyChunkLoadedOrUnloaded();

        void UpdateChunkDataCache();

        void FreeChunkVertexBuffer(Chunk &chunk);

        void FreeChunkVoxelDataBuffer(Chunk &chunk);

    private:
        static constexpr glm::u32 MAXIMUM_VERTICES_IN_WORLD = 36 * (SPIRE_VOXEL_CHUNK_VOLUME * 48);
        static constexpr glm::u32 MAXIMUM_LOADED_CHUNKS = 384 * 3;

        VoxelWorld &m_world;

        Spire::RenderingManager &m_renderingManager;
        Delegate<WorldEditRequiredChanges> m_onWorldEditedDelegate;
        BufferAllocator m_chunkVertexBufferAllocator;
        BufferAllocator m_chunkVoxelDataBufferAllocator;
        std::unique_ptr<Spire::PerImageBuffer> m_chunkDatasBuffer;
        std::vector<bool> m_dirtyChunkDataBuffers; // if {true,false,false} it means we need to update buffer 0 on swapchain image index 0
        std::vector<ChunkData> m_latestCachedChunkData;
        std::unordered_set<glm::ivec3> m_editedChunks;
        std::unique_ptr<ChunkMesherManager> m_mesher;
        std::unordered_map<std::shared_ptr<Chunk>, std::vector<VertexData> > m_remeshedChunks; // chunks that have been remeshed and we need to upload the new mesh
    };
} // SpireVoxel
