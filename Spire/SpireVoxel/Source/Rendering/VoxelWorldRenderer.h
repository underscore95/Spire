#pragma once

#include "VoxelRenderer.h"
#include "Chunk/Chunk.h"
#include "Chunk/meshing/ChunkMesh.h"
#include "Chunk/meshing/ChunkMesher.h"
#include "Chunk/meshing/WorldEditRequiredChanges.h"

namespace SpireVoxel {
    class VoxelWorld;

    class VoxelWorldRenderer {
        friend class VoxelWorld;

    public:
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
        // Upload chunk mesh to GPU
        // voxelDataMemory - mapped memory for m_chunkVoxelDataBufferAllocator
        // chunkVertexBufferMemory - mapped memory for m_chunkVertexBufferAllocator
        // futures - some steps are parallelized, void futures will be pushed to this vector and the function is only complete once all futures return.
        // Until futures haven't returned:
        // chunk, chunkVertexBufferMemory, mesh, and voxelDataMemory must be kept alive
        // the chunks vertex buffer and voxel data allocations must not be changed
        void UploadChunkMesh(Chunk &chunk, ChunkMesh& mesh, Spire::BufferManager::MappedMemory &voxelDataMemory, Spire::BufferManager::MappedMemory &chunkVertexBufferMemory, std::vector<std
                             ::future<void>> &futures);

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
        std::unique_ptr<ChunkMesher> m_chunkMesher;
    };
} // SpireVoxel
