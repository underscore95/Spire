#include "VoxelWorldRenderer.h"

#include "Chunk/VoxelWorld.h"
#include "Chunk/Meshing/ChunkMesher.h"
#include "Rendering/Memory/BufferManager.h"
#include "Utils/ThreadPool.h"
#include "../../Assets/Shaders/PushConstants.h"
#include "Edits/BasicVoxelEdit.h"
#include "Edits/BasicVoxelEdit.h"
#include "Edits/BasicVoxelEdit.h"
#include "Edits/BasicVoxelEdit.h"

namespace SpireVoxel {
    VoxelWorldRenderer::VoxelWorldRenderer(VoxelWorld &world,
                                           Spire::RenderingManager &renderingManager,
                                           const std::function<void()> &recreatePipelineCallback,
                                           bool isProfilingMeshing)
        : m_world(world),
          m_renderingManager(renderingManager),
          m_onWorldEditedDelegate(),
          m_chunkVertexBufferAllocator(m_renderingManager, recreatePipelineCallback, sizeof(VertexData), m_renderingManager.GetSwapchain().GetNumImages(),
                                       sizeof(VertexData) * (1024 * 1024 * 32), 1, true),
          m_chunkVoxelDataBufferAllocator(m_renderingManager, recreatePipelineCallback, sizeof(GPUChunkVoxelData), m_renderingManager.GetSwapchain().GetNumImages(),
                                          sizeof(GPUChunkVoxelData) * 1024, 1, true) {
        Spire::info("Allocated {} mb BufferAllocator on GPU to store world vertices", m_chunkVertexBufferAllocator.GetTotalSize() / 1024 / 1024);
        Spire::info("Allocated {} mb BufferAllocator on GPU to store world voxel data", m_chunkVoxelDataBufferAllocator.GetTotalSize() / 1024 / 1024);


        m_chunkDatasBuffer = m_renderingManager.GetBufferManager().CreatePerImageStorageBuffers(sizeof(ChunkData) * MAXIMUM_LOADED_CHUNKS, MAXIMUM_LOADED_CHUNKS, nullptr,
                                                                                        VK_BUFFER_USAGE_2_INDIRECT_BUFFER_BIT);
        Spire::info("Allocated {} kb buffer for each swapchain image on GPU to store chunk datas", sizeof(ChunkData) * MAXIMUM_LOADED_CHUNKS / 1024);

        m_dirtyChunkDataBuffers.resize(renderingManager.GetSwapchain().GetNumImages());

        m_chunkMesher = std::make_unique<ChunkMesher>(m_world, m_chunkVertexBufferAllocator, m_chunkVoxelDataBufferAllocator, isProfilingMeshing);
    }

    void VoxelWorldRenderer::Render(glm::u32 swapchainImageIndex, glm::vec3 cameraPos) {
        HandleChunkEdits(cameraPos);

        // if empty we aren't issuing render commands so don't need to update the gpu buffer
        if (!m_latestCachedChunkData.empty() && m_dirtyChunkDataBuffers[swapchainImageIndex]) {
            m_dirtyChunkDataBuffers[swapchainImageIndex] = false;

            const glm::u32 requiredBufferSize = sizeof(m_latestCachedChunkData[0]) * m_latestCachedChunkData.size();
            m_renderingManager.GetBufferManager().UpdateBuffer(m_chunkDatasBuffer->GetBuffer(swapchainImageIndex), m_latestCachedChunkData.data(), requiredBufferSize, 0);
        }

        m_chunkVertexBufferAllocator.Render();
        m_chunkVoxelDataBufferAllocator.Render();
    }

    DelegateSubscribers<> &VoxelWorldRenderer::GetOnWorldEditSubscribers() {
        return m_onWorldEditedDelegate;
    }

    void VoxelWorldRenderer::CmdRender(VkCommandBuffer commandBuffer, glm::u32 swapchainImage, const Spire::Pipeline &pipeline) const {
        if (!m_latestCachedChunkData.empty()) {
            PushConstantsData pushConstants = CreatePushConstants();
            pipeline.CmdSetPushConstants(commandBuffer, &pushConstants, sizeof(PushConstantsData));
            vkCmdDrawIndirect(commandBuffer, m_chunkDatasBuffer->GetBuffer(swapchainImage).Buffer, offsetof(ChunkData, CPU_DrawCommandParams), m_latestCachedChunkData.size(),
                              sizeof(ChunkData));
        }
    }

    void VoxelWorldRenderer::PushDescriptors(Spire::PerImageDescriptorSetLayout &perFrameSet, Spire::DescriptorSetLayout &chunkVertexBuffersLayout) {
        chunkVertexBuffersLayout.push_back(
            m_chunkVertexBufferAllocator.CreateDescriptor(SPIRE_VOXEL_SHADER_BINDINGS_CONSTANT_CHUNK_BINDING, VK_SHADER_STAGE_VERTEX_BIT, "World Vertex Buffer"));
        chunkVertexBuffersLayout.push_back(
            m_chunkVoxelDataBufferAllocator.
            CreateDescriptor(SPIRE_VOXEL_SHADER_BINDINGS_CONSTANT_CHUNK_VOXEL_DATA_BINDING, VK_SHADER_STAGE_FRAGMENT_BIT, "World Voxel Data Buffer"));

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

    void VoxelWorldRenderer::NotifyChunkEdited(const Chunk &chunk) {
        assert(m_world.IsLoaded(chunk));
        assert(m_world.GetLoadedChunk(chunk.ChunkPosition) == &chunk);
        m_editedChunks.insert(chunk.ChunkPosition);
    }

    void VoxelWorldRenderer::HandleChunkEdits(glm::vec3 cameraPos) {
        if (m_chunkMesher->HandleChunkEdits(m_editedChunks, cameraPos)) {
            UpdateChunkDatasBuffer();
            m_onWorldEditedDelegate.Broadcast();
        }
    }

    void VoxelWorldRenderer::NotifyChunkLoadedOrUnloaded() {
        UpdateChunkDatasBuffer();
        m_onWorldEditedDelegate.Broadcast();
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

    PushConstantsData VoxelWorldRenderer::CreatePushConstants() const {
        return PushConstantsData{
            .NumVerticesPerBuffer = m_chunkVertexBufferAllocator.GetNumElementsPerInternalBuffer()
        };
    }
} // SpireVoxel
