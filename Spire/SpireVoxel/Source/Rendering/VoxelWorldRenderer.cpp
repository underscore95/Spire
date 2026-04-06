#include "VoxelWorldRenderer.h"

#include "Chunk/VoxelWorld.h"
#include "Chunk/Meshing/ChunkMesher.h"
#include "Rendering/Memory/BufferManager.h"
#include "Utils/ThreadPool.h"
#include "../../Assets/Shaders/PushConstants.h"
#include "Chunk/ChunkDrawParams.h"
#include "Utils/IVoxelCamera.h"
#include "Utils/MathsUtils.h"

namespace SpireVoxel {
    VoxelWorldRenderer::VoxelWorldRenderer(
        VoxelWorld &world,
        Spire::RenderingManager &renderingManager,
        const std::function<void()> &recreatePipelineCallback,
        const IVoxelCamera &camera,
        const VoxelWorld::Settings &settings
    )
        : m_world(world),
          m_renderingManager(renderingManager),
          m_onWorldEditedDelegate(),
          m_chunkVertexBufferAllocator(m_renderingManager, recreatePipelineCallback, sizeof(VertexData), m_renderingManager.GetSwapchain().GetNumImages(),
                                       sizeof(VertexData) * (1024 * 1024 * 32), 1, true),
          m_chunkVoxelDataBufferAllocator(m_renderingManager, recreatePipelineCallback, sizeof(VoxelType), m_renderingManager.GetSwapchain().GetNumImages(),
                                          1024 * 1024 * 128, 1, true),
          m_chunkAOBufferAllocator(m_renderingManager, recreatePipelineCallback, sizeof(glm::u32), m_renderingManager.GetSwapchain().GetNumImages(),
                                   1024 * 1024 * 128, 1, true),
          m_camera(camera),
          m_settings(settings) {
        Spire::info("Allocated {} mb BufferAllocator on GPU to store world vertices", m_chunkVertexBufferAllocator.GetTotalSize() / 1024 / 1024);
        Spire::info("Allocated {} mb BufferAllocator on GPU to store world voxel data", m_chunkVoxelDataBufferAllocator.GetTotalSize() / 1024 / 1024);
        Spire::info("Allocated {} mb BufferAllocator on GPU to store world voxel data", m_chunkAOBufferAllocator.GetTotalSize() / 1024 / 1024);
        if (!settings.AllowFrustumCulling) {
            Spire::warn("Frustum culling is disabled!");
        }
        if (!settings.AllowBackfaceCulling) {
            Spire::warn("Backface culling is disabled!");
        }

        m_chunkDatasBuffer = m_renderingManager.GetBufferManager().CreatePerImageStorageBuffers(
            sizeof(ChunkData) * MAXIMUM_LOADED_CHUNKS,
            MAXIMUM_LOADED_CHUNKS,
            nullptr,
            VK_BUFFER_USAGE_2_INDIRECT_BUFFER_BIT
        );
        m_chunkDrawCommandsBuffer = m_renderingManager.GetBufferManager().CreatePerImageStorageBuffers(
            sizeof(ChunkDrawParams) * MAXIMUM_LOADED_CHUNKS,
            MAXIMUM_LOADED_CHUNKS,
            nullptr,
            VK_BUFFER_USAGE_2_INDIRECT_BUFFER_BIT
        );
        Spire::info("Allocated {} kb buffer for each swapchain image on GPU to store chunk datas", sizeof(ChunkData) * MAXIMUM_LOADED_CHUNKS / 1024);

        m_dirtyChunkDataBuffers.resize(renderingManager.GetSwapchain().GetNumImages());

        m_chunkMesher = std::make_unique<ChunkMesher>(m_world, m_chunkVertexBufferAllocator, m_chunkVoxelDataBufferAllocator, m_chunkAOBufferAllocator, settings);
    }

    void VoxelWorldRenderer::Render(glm::u32 swapchainImageIndex, glm::vec3 cameraPos) {
        HandleChunkEdits(cameraPos);

        // Frustum culling
        if (m_cameraInfoLastFrame != m_camera.GetCameraInfo()) {
            m_cameraInfoLastFrame = m_camera.GetCameraInfo();
            UpdateChunkDatasBuffer();
        }

        // if empty we aren't issuing render commands so don't need to update the gpu buffer
        if (!m_latestCachedChunkData.empty() && m_dirtyChunkDataBuffers[swapchainImageIndex]) {
            m_dirtyChunkDataBuffers[swapchainImageIndex] = false;

            const glm::u32 chunkDataWriteSize = sizeof(m_latestCachedChunkData[0]) * m_latestCachedChunkData.size();
            m_renderingManager.GetBufferManager().UpdateBuffer(
                m_chunkDatasBuffer->GetBuffer(swapchainImageIndex),
                m_latestCachedChunkData.data(),
                chunkDataWriteSize,
                0
            );

            const glm::u32 chunkDrawParamsWriteSize = sizeof(m_latestCachedChunkDrawCommands[0]) * m_latestCachedChunkDrawCommands.size();
            m_renderingManager.GetBufferManager().UpdateBuffer(
                m_chunkDrawCommandsBuffer->GetBuffer(swapchainImageIndex),
                m_latestCachedChunkDrawCommands.data(),
                chunkDrawParamsWriteSize,
                0
            );
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
            vkCmdDrawIndirect(
                commandBuffer,
                m_chunkDrawCommandsBuffer->GetBuffer(swapchainImage).Buffer,
                0,
                m_latestCachedChunkDrawCommands.size() * ChunkDrawParams::COMMANDS_PER_CHUNK,
                ChunkDrawParams::STRIDE
            );
        }
    }

    void VoxelWorldRenderer::PushDescriptors(Spire::PerImageDescriptorSetLayout &perFrameSet, Spire::DescriptorSetLayout &chunkVertexBuffersLayout) {
        chunkVertexBuffersLayout.push_back(
            m_chunkVertexBufferAllocator.CreateDescriptor(SPIRE_VOXEL_SHADER_BINDINGS_CONSTANT_CHUNK_BINDING, VK_SHADER_STAGE_VERTEX_BIT, "World Vertex Buffer"));
        chunkVertexBuffersLayout.push_back(
            m_chunkVoxelDataBufferAllocator.
            CreateDescriptor(SPIRE_VOXEL_SHADER_BINDINGS_CONSTANT_CHUNK_VOXEL_DATA_BINDING, VK_SHADER_STAGE_FRAGMENT_BIT, "World Voxel Data Buffer"));
        chunkVertexBuffersLayout.push_back(
            m_chunkAOBufferAllocator.
            CreateDescriptor(SPIRE_VOXEL_SHADER_BINDINGS_AO_DATA_BINDING, VK_SHADER_STAGE_FRAGMENT_BIT, "World AO Data Buffer"));

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
        std::unique_lock lock(m_chunkEditNotifyMutex);
        assert(m_world.IsLoaded(chunk));
        assert(m_world.TryGetLoadedChunk(chunk.ChunkPosition) == &chunk);
        m_editedChunks.insert(chunk.ChunkPosition);
    }

    void VoxelWorldRenderer::HandleChunkEdits(glm::vec3 cameraPos) {
        std::unique_lock lock(m_chunkEditNotifyMutex);
        if (m_chunkMesher->HandleChunkEdits(m_editedChunks, cameraPos)) {
            UpdateChunkDatasBuffer();
            m_onWorldEditedDelegate.Broadcast();
        }
    }

    glm::u32 VoxelWorldRenderer::NumEditedChunks() const {
        return m_editedChunks.size();
    }

    glm::u32 VoxelWorldRenderer::GetNumChunksOutsideFrustum() const {
        return m_numChunksOutsideFrustum;
    }

    glm::u32 VoxelWorldRenderer::GetNumNonEmptyChunks() const {
        return m_numNonEmptyChunks;
    }

    glm::u32 VoxelWorldRenderer::GetNumBackfaceCulledFaces() const {
        return m_numBackfaceCulledFaces;
    }

    glm::u32 VoxelWorldRenderer::GetNumNonBackfaceCulledFaces() const {
        return m_numNonBackfaceCulledFaces;
    }

    glm::u32 VoxelWorldRenderer::NumRenderedFaces() const {
        return m_numRenderedFaces;
    }

    glm::u32 VoxelWorldRenderer::NumFaces() const {
        return m_numFaces;
    }

    void VoxelWorldRenderer::NotifyChunkLoadedOrUnloaded() {
        std::unique_lock lock(m_chunkEditNotifyMutex);
        UpdateChunkDatasBuffer();
        m_onWorldEditedDelegate.Broadcast();
    }

    void VoxelWorldRenderer::UpdateChunkDataCache() {
        m_latestCachedChunkData.clear();
        m_latestCachedChunkDrawCommands.clear();

        Spire::Frustum cameraFrustum = m_camera.CalculateFrustum();
        m_numChunksOutsideFrustum = 0;
        m_numNonEmptyChunks = 0;
        m_numBackfaceCulledFaces = 0;
        m_numNonBackfaceCulledFaces = 0;
        m_numRenderedFaces = 0;
        m_numFaces = 0;

        for (const auto &[_, chunk] : m_world) {
            auto chunkIndex = static_cast<glm::u32>(m_latestCachedChunkData.size());
            if (chunk->VertexAllocation.Size == 0) continue;
            m_numNonEmptyChunks++;

            m_latestCachedChunkData.push_back(chunk->GenerateChunkData());
            m_latestCachedChunkDrawCommands.push_back(chunk->GenerateDrawParams(chunkIndex));

            glm::vec3 worldPosition = VoxelWorld::GetWorldVoxelPositionInChunk(chunk->ChunkPosition, {0, 0, 0});
            float cameraScale = m_camera.GetCameraInfo().Scale;
            worldPosition *= cameraScale;

            bool shouldRenderChunk = !m_settings.AllowFrustumCulling || cameraFrustum.IsBoxVisible(worldPosition, worldPosition + SPIRE_VOXEL_CHUNK_DIMENSIONS_FLOAT * cameraScale);
            m_numFaces += chunk->TotalVertices / Chunk::VERTICES_PER_FACE;

            assert(ChunkDrawParams::COMMANDS_PER_CHUNK == SPIRE_VOXEL_NUM_FACES);
            for (glm::u32 face = 0; face < SPIRE_VOXEL_NUM_FACES; face++) {
                // Essentially, if the player's X coordinate is greater than the maximum X coordinate of the chunk, don't render any negative X faces
                // This is back face culling: https://en.wikipedia.org/wiki/Back-face_culling
                // but we can simplify the algorithm since every face is axis aligned

                glm::vec3 faceNormal = FaceToDirectionFloat(face);
                glm::vec3 centerOfOppositeFace = worldPosition - faceNormal * (SPIRE_VOXEL_CHUNK_SIZE * cameraScale);

                glm::u32 index = face / 2; // x, y, or z coordinate
                bool shouldRenderFace = IsFaceOnNegativeAxis(face)
                                            ? centerOfOppositeFace[index] >= m_camera.GetPosition()[index]
                                            : centerOfOppositeFace[index] <= m_camera.GetPosition()[index];
                if (!m_settings.AllowBackfaceCulling) shouldRenderFace = true;

                m_latestCachedChunkDrawCommands.back().Commands[face].instanceCount = shouldRenderChunk && shouldRenderFace ? 1 : 0;
                if (m_latestCachedChunkDrawCommands.back().Commands[face].instanceCount == 1) {
                    m_numRenderedFaces += chunk->NumVertices[face] / Chunk::VERTICES_PER_FACE;
                }

                if (shouldRenderChunk) {
                    if (shouldRenderFace) m_numNonBackfaceCulledFaces++;
                    else m_numBackfaceCulledFaces++;
                }
            }
            if (!shouldRenderChunk) m_numChunksOutsideFrustum++;
        }
    }

    void VoxelWorldRenderer::FreeChunkBuffers(Chunk &chunk) {
        if (chunk.VertexAllocation.Size > 0) {
            m_chunkVertexBufferAllocator.ScheduleFreeAllocation(chunk.VertexAllocation);
            chunk.VertexAllocation = {};
        }
        if (chunk.VoxelDataAllocation.Size > 0) {
            m_chunkVoxelDataBufferAllocator.ScheduleFreeAllocation(chunk.VoxelDataAllocation);
            chunk.VoxelDataAllocation = {};
        }
        if (chunk.AODataAllocation.Size > 0) {
            m_chunkAOBufferAllocator.ScheduleFreeAllocation(chunk.AODataAllocation);
            chunk.AODataAllocation = {};
        }
    }

    PushConstantsData VoxelWorldRenderer::CreatePushConstants() const {
        return PushConstantsData{
            .NumVerticesPerBuffer = m_chunkVertexBufferAllocator.GetNumElementsPerInternalBuffer()
        };
    }
} // SpireVoxel
