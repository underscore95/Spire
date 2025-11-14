#include "VoxelWorldRenderer.h"

#include "Chunk/VoxelWorld.h"

namespace SpireVoxel {
    VoxelWorldRenderer::VoxelWorldRenderer(VoxelWorld &world,
                                           Spire::RenderingManager &renderingManager)
        : m_world(world),
          m_renderingManager(renderingManager),
          m_onWorldEditedDelegate(),
          m_chunkVertexBufferAllocator(m_renderingManager, sizeof(VertexData), m_renderingManager.GetSwapchain().GetNumImages(), sizeof(VertexData) * 10000) {
        UpdateChunkDatasBuffer();

        m_chunkDatasBuffer = m_renderingManager.GetBufferManager().CreateStorageBuffer(nullptr, sizeof(ChunkData) * 4096, sizeof(ChunkData));
    }

    VoxelWorldRenderer::~VoxelWorldRenderer() {
        m_renderingManager.GetBufferManager().DestroyBuffer(m_chunkDatasBuffer);
    }

    void VoxelWorldRenderer::Update() {
        m_chunkVertexBufferAllocator.Update();
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

    void VoxelWorldRenderer::PushDescriptors(Spire::DescriptorSetLayout &constantDataLayout, Spire::DescriptorSetLayout &chunkVertexBuffersLayout) {
        Spire::Descriptor descriptor = m_chunkVertexBufferAllocator.GetDescriptor(SPIRE_VOXEL_SHADER_BINDINGS_CONSTANT_CHUNK_BINDING, "World Vertex Buffer");
        chunkVertexBuffersLayout.push_back(descriptor);

        assert(m_chunkDatasBuffer.Buffer != VK_NULL_HANDLE);
        descriptor = {
            .ResourceType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .Binding = SPIRE_VOXEL_SHADER_BINDINGS_CHUNK_DATA_SSBO_BINDING,
            .Stages = VK_SHADER_STAGE_VERTEX_BIT,
            .Resources = {{.Buffer = &m_chunkDatasBuffer}},
#ifndef NDEBUG
            .DebugName = "Chunk Datas",
#endif
        };
        constantDataLayout.push_back(descriptor);
    }

    void VoxelWorldRenderer::UpdateChunkDatasBuffer() const {
        // get chunk datas
        std::vector<ChunkData> chunkDatas;
        chunkDatas.reserve(m_world.NumLoadedChunks());
        for (const auto &[_, chunk] : m_world) {
            ChunkData data = {
                .NumVertices = chunk.NumVertices,
                .FirstVertex = chunk.Allocation.Start / static_cast<glm::u32>(sizeof(VertexData))
            };
            if (data.NumVertices == 0 || chunk.Allocation.Size == 0) continue;

            chunkDatas.push_back(data);
        }

        // update buffer
        if (!chunkDatas.empty()) {
            const glm::u32 requiredBufferSize = sizeof(chunkDatas[0]) * chunkDatas.size();
            m_renderingManager.GetBufferManager().UpdateBuffer(m_chunkDatasBuffer, chunkDatas.data(), requiredBufferSize, 0);
        }
    }

    void VoxelWorldRenderer::NotifyChunkEdited(Chunk &chunk) {
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
            m_chunkVertexBufferAllocator.FreeAllocation(oldAllocation.Start);
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
            m_renderingManager.GetBufferManager().UpdateBuffer(m_chunkDatasBuffer, &data, sizeof(data), chunkIndex * sizeof(data));
        }

        changes.RecreateOnlyCommandBuffers = true;

        m_onWorldEditedDelegate.Broadcast(changes);
        Spire::info("Regenerated chunk ({}, {}) with {} vertices", chunk.ChunkPosition.x, chunk.ChunkPosition.y, chunk.NumVertices);
    }

    void VoxelWorldRenderer::NotifyChunkLoadedOrUnloaded() {
        UpdateChunkDatasBuffer();
        m_onWorldEditedDelegate.Broadcast({true, false});
    }
} // SpireVoxel
