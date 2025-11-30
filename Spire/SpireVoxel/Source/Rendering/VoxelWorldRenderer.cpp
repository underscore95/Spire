#include "VoxelWorldRenderer.h"

#include "Chunk/VoxelWorld.h"

namespace SpireVoxel {
    VoxelWorldRenderer::VoxelWorldRenderer(VoxelWorld &world,
                                           Spire::RenderingManager &renderingManager)
        : m_world(world),
          m_renderingManager(renderingManager),
          m_onWorldEditedDelegate(),
          m_chunkVertexBufferAllocator(m_renderingManager, sizeof(VertexData), m_renderingManager.GetSwapchain().GetNumImages(), sizeof(VertexData) * MAXIMUM_VERTICES_IN_WORLD),
          m_chunkVoxelDataBufferAllocator(m_renderingManager, sizeof(GPUChunkVoxelData), m_renderingManager.GetSwapchain().GetNumImages(),
                                          sizeof(GPUChunkVoxelData) * MAXIMUM_LOADED_CHUNKS) {
        Spire::info("Allocated {} mb BufferAllocator on GPU to store world vertices", sizeof(VertexData) * MAXIMUM_VERTICES_IN_WORLD / 1024 / 1024);
        Spire::info("Allocated {} mb BufferAllocator on GPU to store world voxel data", sizeof(GPUChunkVoxelData) * MAXIMUM_LOADED_CHUNKS / 1024 / 1024);

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
        chunkVertexBuffersLayout.push_back(
            m_chunkVertexBufferAllocator.GetDescriptor(SPIRE_VOXEL_SHADER_BINDINGS_CONSTANT_CHUNK_BINDING, VK_SHADER_STAGE_VERTEX_BIT, "World Vertex Buffer"));
        chunkVertexBuffersLayout.push_back(
            m_chunkVoxelDataBufferAllocator.GetDescriptor(SPIRE_VOXEL_SHADER_BINDINGS_CONSTANT_CHUNK_VOXEL_DATA_BINDING, VK_SHADER_STAGE_FRAGMENT_BIT, "World Voxel Data Buffer"));

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

        for (std::size_t i = 0; i < m_dirtyChunkDataBuffers.size(); i++) {
            m_dirtyChunkDataBuffers[i] = true;
        }
    }

    void VoxelWorldRenderer::NotifyChunkEdited(Chunk &chunk) {
        assert(m_world.IsLoaded(chunk));
        assert(m_world.GetLoadedChunk(chunk.ChunkPosition) == &chunk);
        m_editedChunks.insert(chunk.ChunkPosition);
    }

    void VoxelWorldRenderer::HandleChunkEdits() {
        WorldEditRequiredChanges changes = {false, false};
        for (auto &chunkPos : m_editedChunks) {
            Chunk *chunk = m_world.GetLoadedChunk(chunkPos);
            if (!chunk) continue;

            // write the new mesh
            bool wasPreviousAllocation = false; {
                const BufferAllocator::Allocation oldAllocation = chunk->VertexAllocation;
                wasPreviousAllocation = oldAllocation.Size > 0;
                chunk->VertexAllocation = {};

                std::vector<VertexData> vertexData = chunk->GenerateMesh();

                if (!vertexData.empty()) {
                    std::optional alloc = m_chunkVertexBufferAllocator.Allocate(vertexData.size() * sizeof(VertexData));
                    if (alloc) {
                        chunk->VertexAllocation = *alloc;

                        // write the mesh into the vertex buffer
                        m_chunkVertexBufferAllocator.Write(chunk->VertexAllocation, vertexData.data(), vertexData.size() * sizeof(VertexData));
                    } else {
                        Spire::error("Chunk vertex data allocation failed");
                    }
                }

                if (oldAllocation.Size > 0) {
                    m_chunkVertexBufferAllocator.ScheduleFreeAllocation(oldAllocation.Start);
                }

                chunk->NumVertices = vertexData.size();
            }

            // write the voxel data
            {
                const BufferAllocator::Allocation oldAllocation = chunk->VoxelDataAllocation;

                if (chunk->NumVertices > 0) {
                    std::optional alloc = m_chunkVoxelDataBufferAllocator.Allocate(sizeof(GPUChunkVoxelData));
                    if (alloc) {
                        chunk->VoxelDataAllocation = *alloc;

                        // write the voxel data
                        m_chunkVoxelDataBufferAllocator.Write(chunk->VoxelDataAllocation, chunk->VoxelData.data(), sizeof(GPUChunkVoxelData));
                    } else {
                        // allocation failed, need to free the vertex allocation
                        Spire::error("Chunk voxel data allocation failed, deallocating vertex buffer");
                        m_chunkVertexBufferAllocator.ScheduleFreeAllocation(chunk->VertexAllocation);
                        chunk->VertexAllocation = {};
                    }
                }

                if (oldAllocation.Size > 0) {
                    m_chunkVoxelDataBufferAllocator.ScheduleFreeAllocation(oldAllocation.Start);
                }
            }

            // write the chunk data
            if (!wasPreviousAllocation) {
                UpdateChunkDatasBuffer();
                changes.RecreatePipeline = true;
            } else {
                glm::u32 chunkIndex = 0;
                for (auto &[_,c] : m_world) {
                    if (c.get() == chunk) break;
                    if (c->VertexAllocation.Size == 0) continue;
                    chunkIndex++;
                }

                m_latestCachedChunkData[chunkIndex] = chunk->GenerateChunkData(chunkIndex);
                for (std::size_t i = 0; i < m_dirtyChunkDataBuffers.size(); i++) {
                    m_dirtyChunkDataBuffers[i] = true;
                }
            }
        }

        if (!m_editedChunks.empty()) {
            changes.RecreateOnlyCommandBuffers = true;
            m_onWorldEditedDelegate.Broadcast(changes);
            m_editedChunks.clear();
        }
    }

    void VoxelWorldRenderer::NotifyChunkLoadedOrUnloaded() {
        UpdateChunkDatasBuffer();
        m_onWorldEditedDelegate.Broadcast({true, false});
    }

    void VoxelWorldRenderer::UpdateChunkDataCache() {
        m_latestCachedChunkData.clear();
        for (const auto &[_, chunk] : m_world) {
            auto chunkIndex = static_cast<glm::u32>(m_latestCachedChunkData.size());
            if (chunk->VertexAllocation.Size == 0) continue;

            m_latestCachedChunkData.push_back(chunk->GenerateChunkData(chunkIndex));
        }
    }

    void VoxelWorldRenderer::FreeChunkVertexBuffer(Chunk &chunk) {
        if (chunk.VertexAllocation.Size > 0) {
            m_chunkVertexBufferAllocator.ScheduleFreeAllocation(chunk.VertexAllocation);
            chunk.VertexAllocation = {};
        }
    }

    void VoxelWorldRenderer::FreeChunkVoxelDataBuffer(Chunk &chunk) {
        if (chunk.VoxelDataAllocation.Size > 0) {
            m_chunkVoxelDataBufferAllocator.ScheduleFreeAllocation(chunk.VoxelDataAllocation);
            chunk.VoxelDataAllocation = {};
        }
    }
} // SpireVoxel
