#include "VoxelWorldRenderer.h"

#include "Chunk/VoxelWorld.h"

namespace SpireVoxel {
    VoxelWorldRenderer::VoxelWorldRenderer(VoxelWorld &world,
                                           Spire::RenderingManager &renderingManager)
        : m_world(world),
          m_renderingManager(renderingManager),
          m_onWorldEditedDelegate(),
          m_chunkVertexBufferAllocator(m_renderingManager, sizeof(VertexData), m_renderingManager.GetSwapchain().GetNumImages(), sizeof(VertexData) * MAXIMUM_VERTICES_IN_WORLD) {
        Spire::info("Allocated {} mb BufferAllocator on GPU to store world vertices", sizeof(VertexData) * MAXIMUM_VERTICES_IN_WORLD / 1024 / 1024);

        m_chunkDatasBuffer = m_renderingManager.GetBufferManager().CreateStorageBuffers(sizeof(ChunkData) * MAXIMUM_LOADED_CHUNKS, MAXIMUM_LOADED_CHUNKS, nullptr);
        Spire::info("Allocated {} kb buffer for each swapchain image on GPU to store chunk datas", sizeof(ChunkData) * MAXIMUM_LOADED_CHUNKS / 1024);
    }

    void VoxelWorldRenderer::Render(glm::u32 swapchainImageIndex) {
        // if empty we aren't issuing render commands so don't need to update the gpu buffer
        if (!m_latestCachedChunkData.empty()) {
            const glm::u32 requiredBufferSize = sizeof(m_latestCachedChunkData[0]) * m_latestCachedChunkData.size();
            m_renderingManager.GetBufferManager().UpdateBuffer(m_chunkDatasBuffer->GetBuffer(swapchainImageIndex), m_latestCachedChunkData.data(), requiredBufferSize, 0);
        }

        m_chunkVertexBufferAllocator.Render();
    }

    DelegateSubscribers<VoxelWorldRenderer::WorldEditRequiredChanges> &VoxelWorldRenderer::GetOnWorldEditSubscribers() {
        return m_onWorldEditedDelegate;
    }

    void VoxelWorldRenderer::CmdRender(VkCommandBuffer commandBuffer) const {
        glm::u32 numVerticesToRender = 0;
        for (auto &[_,chunk] : m_world) {
            numVerticesToRender = std::max(numVerticesToRender, chunk.NumVertices);
        }

        vkCmdDraw(commandBuffer, numVerticesToRender, m_world.GetNumLoadedChunks(), 0, 0);
    }

    void VoxelWorldRenderer::PushDescriptors(Spire::PerImageDescriptorSetLayout &perFrameSet, Spire::DescriptorSetLayout &chunkVertexBuffersLayout) {
        chunkVertexBuffersLayout.push_back(m_chunkVertexBufferAllocator.GetDescriptor(SPIRE_VOXEL_SHADER_BINDINGS_CONSTANT_CHUNK_BINDING, "World Vertex Buffer"));

        Spire::PerImageDescriptor chunkDatasDescriptor = m_renderingManager.GetDescriptorCreator().CreatePerImageStorageBuffer(
            SPIRE_VOXEL_SHADER_BINDINGS_CHUNK_DATA_SSBO_BINDING,
            *m_chunkDatasBuffer,
            VK_SHADER_STAGE_VERTEX_BIT,
            "Chunk Datas"
        );

        perFrameSet.push_back(chunkDatasDescriptor);
    }

    void VoxelWorldRenderer::UpdateChunkDatasBuffer() {
        UpdateChunkDataCache();

        for (size_t i = 0; i < m_dirtyChunkDataBuffers.size(); i++) {
            m_dirtyChunkDataBuffers[i] = true;
        }
    }

    void VoxelWorldRenderer::NotifyChunkEdited(Chunk &chunk) {
        assert(m_world.IsLoaded(chunk));

        std::vector<VertexData> vertexData = chunk.GenerateMesh();

        BufferAllocator::Allocation oldAllocation = chunk.Allocation;
        if (!vertexData.empty()) {
            chunk.Allocation = m_chunkVertexBufferAllocator.Allocate(vertexData.size() * sizeof(VertexData));

            // write the mesh into the vertex buffer
            m_chunkVertexBufferAllocator.Write(chunk.Allocation, vertexData.data(), vertexData.size() * sizeof(VertexData));
        } else {
            chunk.Allocation = {};
        }

        if (oldAllocation.Size > 0) {
            m_chunkVertexBufferAllocator.ScheduleFreeAllocation(oldAllocation.Start);
        }

        // write the chunk data
        chunk.NumVertices = vertexData.size();

        WorldEditRequiredChanges changes = {false, false};
        if (oldAllocation.Size == 0) {
            UpdateChunkDatasBuffer();
            changes.RecreatePipeline = true;
        } else {
            glm::u32 chunkIndex = 0;
            for (auto &[_,c] : m_world) {
                if (&c == &chunk) break;
                if (c.Allocation.Size == 0) continue;
                chunkIndex++;
            }

            ChunkData data = {
                .NumVertices = static_cast<glm::u32>(vertexData.size()),
                .FirstVertex = chunk.Allocation.Start / static_cast<glm::u32>(sizeof(VertexData))
            };
            m_latestCachedChunkData[chunkIndex] = data;
            for (size_t i = 0; i < m_dirtyChunkDataBuffers.size(); i++) {
                m_dirtyChunkDataBuffers[i] = true;
            }
        }

        changes.RecreateOnlyCommandBuffers = true;

        m_onWorldEditedDelegate.Broadcast(changes);
        //Spire::info("Regenerated chunk ({}, {}) with {} vertices", chunk.ChunkPosition.x, chunk.ChunkPosition.y, chunk.NumVertices);
    }

    void VoxelWorldRenderer::NotifyChunkLoadedOrUnloaded() {
        UpdateChunkDatasBuffer();
        m_onWorldEditedDelegate.Broadcast({true, false});
    }

    void VoxelWorldRenderer::UpdateChunkDataCache() {
        m_latestCachedChunkData.clear();
        for (const auto &[_, chunk] : m_world) {
            ChunkData data = {
                .NumVertices = chunk.NumVertices,
                .FirstVertex = chunk.Allocation.Start / static_cast<glm::u32>(sizeof(VertexData))
            };
            if (data.NumVertices == 0 || chunk.Allocation.Size == 0) continue;

            m_latestCachedChunkData.push_back(data);
        }
    }

    void VoxelWorldRenderer::FreeChunkVertexBuffer(Chunk &chunk) {
        if (chunk.Allocation.Size > 0) {
            m_chunkVertexBufferAllocator.ScheduleFreeAllocation(chunk.Allocation);
            chunk.Allocation = {};
        }
    }
} // SpireVoxel
