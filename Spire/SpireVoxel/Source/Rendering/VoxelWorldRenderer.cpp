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

        m_chunkDatasBuffer = m_renderingManager.GetBufferManager().CreateStorageBuffers(sizeof(ChunkData) * MAXIMUM_LOADED_CHUNKS, MAXIMUM_LOADED_CHUNKS, nullptr,
                                                                                        VK_BUFFER_USAGE_2_INDIRECT_BUFFER_BIT);
        Spire::info("Allocated {} kb buffer for each swapchain image on GPU to store chunk datas", sizeof(ChunkData) * MAXIMUM_LOADED_CHUNKS / 1024);

        m_dirtyChunkDataBuffers.resize(renderingManager.GetSwapchain().GetNumImages());
    }

    void VoxelWorldRenderer::Render(glm::u32 swapchainImageIndex) {
        HandleChunkEdits();

        // if empty we aren't issuing render commands so don't need to update the gpu buffer
        if (!m_latestCachedChunkData.empty() && m_dirtyChunkDataBuffers[swapchainImageIndex]) {
            m_dirtyChunkDataBuffers[swapchainImageIndex] = false;

            const glm::u32 requiredBufferSize = sizeof(m_latestCachedChunkData[0]) * m_latestCachedChunkData.size();
            m_renderingManager.GetBufferManager().UpdateBuffer(m_chunkDatasBuffer->GetBuffer(swapchainImageIndex), m_latestCachedChunkData.data(), requiredBufferSize, 0);
        }

        m_chunkVertexBufferAllocator.Render();
    }

    DelegateSubscribers<VoxelWorldRenderer::WorldEditRequiredChanges> &VoxelWorldRenderer::GetOnWorldEditSubscribers() {
        return m_onWorldEditedDelegate;
    }

    void VoxelWorldRenderer::CmdRender(glm::u32 swapchainImage, VkCommandBuffer commandBuffer) const {
        vkCmdDrawIndirect(commandBuffer, m_chunkDatasBuffer->GetBuffer(swapchainImage).Buffer, offsetof(ChunkData, CPU_DrawCommandParams), m_latestCachedChunkData.size(),
                          sizeof(ChunkData));
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
        assert(m_world.GetLoadedChunk(chunk.ChunkPosition) == &chunk);
        m_editedChunks.push_back(chunk.ChunkPosition);
    }

    void VoxelWorldRenderer::HandleChunkEdits() {
        for (auto &chunkPos : m_editedChunks) {
            Chunk *chunkPtr = m_world.GetLoadedChunk(chunkPos);
            if (!chunkPtr) continue;
            Chunk &chunk = *chunkPtr;
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
                    .CPU_DrawCommandParams = {
                        .vertexCount = static_cast<glm::u32>(vertexData.size()),
                        .instanceCount = 1,
                        .firstVertex = chunk.Allocation.Start / static_cast<glm::u32>(sizeof(VertexData)),
                        .firstInstance = chunkIndex
                    },
                    .ChunkX = chunk.ChunkPosition.x,
                    .ChunkY = chunk.ChunkPosition.y,
                    .ChunkZ = chunk.ChunkPosition.z
                };
                m_latestCachedChunkData[chunkIndex] = data;
                for (std::size_t i = 0; i < m_dirtyChunkDataBuffers.size(); i++) {
                    m_dirtyChunkDataBuffers[i] = true;
                }
            }

            changes.RecreateOnlyCommandBuffers = true;

            m_onWorldEditedDelegate.Broadcast(changes);
            //Spire::info("Regenerated chunk ({}, {}) with {} vertices", chunk.ChunkPosition.x, chunk.ChunkPosition.y, chunk.NumVertices);
        }
        m_editedChunks.clear();
    }

    void VoxelWorldRenderer::NotifyChunkLoadedOrUnloaded() {
        UpdateChunkDatasBuffer();
        m_onWorldEditedDelegate.Broadcast({true, false});
    }

    void VoxelWorldRenderer::UpdateChunkDataCache() {
        m_latestCachedChunkData.clear();
        for (const auto &[_, chunk] : m_world) {
            ChunkData data = {
                .CPU_DrawCommandParams = {
                    .vertexCount = chunk.NumVertices,
                    .instanceCount = 1,
                    .firstVertex = chunk.Allocation.Start / static_cast<glm::u32>(sizeof(VertexData)),
                    .firstInstance = static_cast<glm::u32>(m_latestCachedChunkData.size())
                },
                .ChunkX = chunk.ChunkPosition.x,
                .ChunkY = chunk.ChunkPosition.y,
                .ChunkZ = chunk.ChunkPosition.z
            };
            if (chunk.Allocation.Size == 0) continue;

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
