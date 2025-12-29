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
        chunkVertexBuffersLayout.push_back(
            m_chunkVertexBufferAllocator.GetDescriptor(SPIRE_VOXEL_SHADER_BINDINGS_CONSTANT_CHUNK_BINDING, VK_SHADER_STAGE_VERTEX_BIT, "World Vertex Buffer"));

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
            bool wasPreviousAllocation = false;
            ChunkMesh mesh; {
                const BufferAllocator::Allocation oldAllocation = chunk->VertexAllocation;
                wasPreviousAllocation = oldAllocation.Size > 0;
                chunk->VertexAllocation = {};

                mesh = chunk->GenerateMesh();

                if (!mesh.Vertices.empty()) {
                    std::optional alloc = m_chunkVertexBufferAllocator.Allocate(mesh.Vertices.size() * sizeof(VertexData));
                    if (alloc) {
                        chunk->VertexAllocation = *alloc;

                        // write the mesh into the vertex buffer
                        m_chunkVertexBufferAllocator.Write(chunk->VertexAllocation, mesh.Vertices.data(), mesh.Vertices.size() * sizeof(VertexData));
                    } else {
                        Spire::error("Chunk vertex data allocation failed");
                    }
                }

                if (oldAllocation.Size > 0) {
                    m_chunkVertexBufferAllocator.ScheduleFreeAllocation(oldAllocation.Start);
                }

                chunk->NumVertices = mesh.Vertices.size();
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
} // SpireVoxel
