#include "VoxelWorld.h"

namespace SpireVoxel {
    VoxelWorld::VoxelWorld(Spire::RenderingManager &renderingManager)
        : m_renderingManager(renderingManager),
          m_onWorldEditedDelegate(),
          m_chunkVertexBufferAllocator(m_renderingManager, sizeof(VertexData), m_renderingManager.GetSwapchain().GetNumImages(), sizeof(VertexData) * 10000) {
        CreateOrUpdateChunkDatasBuffer();

        ChunkData dummyChunkData = {};
        m_dummyChunkDataBuffer = renderingManager.GetBufferManager().CreateStorageBuffer(&dummyChunkData, sizeof(dummyChunkData), sizeof(dummyChunkData));
    }

    VoxelWorld::~VoxelWorld() {
        FreeChunkDatasBuffer();
        m_renderingManager.GetBufferManager().DestroyBuffer(m_dummyChunkDataBuffer);
    }

    Chunk &VoxelWorld::LoadChunk(glm::ivec3 chunkPosition) {
        auto it = m_chunks.find(chunkPosition);
        if (it != m_chunks.end()) return it->second;

        m_chunks.try_emplace(chunkPosition, chunkPosition);
        CreateOrUpdateChunkDatasBuffer();
        m_onWorldEditedDelegate.Broadcast({true, false});
        return m_chunks.find(chunkPosition)->second;
    }

    void VoxelWorld::LoadChunks(const std::vector<glm::ivec3> &chunkPositions) {
        bool loadedAnyChunks = false;

        for (auto chunkPosition : chunkPositions) {
            if (m_chunks.contains(chunkPosition)) continue;
            m_chunks.try_emplace(chunkPosition, chunkPosition);
            loadedAnyChunks = true;
        }

        if (loadedAnyChunks) {
            CreateOrUpdateChunkDatasBuffer();
            m_onWorldEditedDelegate.Broadcast({true, false});
        }
    }

    Chunk *VoxelWorld::GetLoadedChunk(glm::ivec3 chunkPosition) {
        auto it = m_chunks.find(chunkPosition);
        return it != m_chunks.end() ? &it->second : nullptr;
    }

    DelegateSubscribers<VoxelWorld::WorldEditRequiredChanges> &VoxelWorld::GetOnWorldEditSubscribers() {
        return m_onWorldEditedDelegate;
    }

    void VoxelWorld::CmdRender(VkCommandBuffer commandBuffer) {
        glm::u32 numVerticesToRender = 0;
        for (auto &[_,chunk] : m_chunks) {
            numVerticesToRender = std::max(numVerticesToRender, chunk.NumVertices);
        }

        Spire::info("Rendering with {} vertices", numVerticesToRender);
        vkCmdDraw(commandBuffer, numVerticesToRender, m_chunks.size(), 0, 0);
    }

    void VoxelWorld::PushDescriptors(Spire::DescriptorSetLayout &constantDataLayout, Spire::DescriptorSetLayout &chunkVertexBuffersLayout) {
        Spire::Descriptor descriptor = m_chunkVertexBufferAllocator.GetDescriptor(SPIRE_VOXEL_SHADER_BINDINGS_CONSTANT_CHUNK_BINDING, "World Vertex Buffer");
        chunkVertexBuffersLayout.push_back(descriptor);

        Spire::VulkanBuffer &chunkDataBuffer = m_chunkDatasBuffer.Buffer != VK_NULL_HANDLE ? m_chunkDatasBuffer : m_dummyChunkDataBuffer;
        assert(chunkDataBuffer.Buffer != VK_NULL_HANDLE);
        descriptor = {
            .ResourceType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .Binding = SPIRE_VOXEL_SHADER_BINDINGS_CHUNK_DATA_SSBO_BINDING,
            .Stages = VK_SHADER_STAGE_VERTEX_BIT,
            .Resources = {{.Buffer = &chunkDataBuffer}},
#ifndef NDEBUG
            .DebugName = "Chunk Datas",
#endif
        };
        constantDataLayout.push_back(descriptor);
    }

    std::size_t VoxelWorld::NumLoadedChunks() const {
        return m_chunks.size();
    }

    void VoxelWorld::CreateOrUpdateChunkDatasBuffer() {
        // get chunk datas
        std::vector<ChunkData> chunkDatas;
        chunkDatas.reserve(m_chunks.size());
        for (const auto &[_, chunk] : m_chunks) {
            ChunkData data = {
                .NumVertices = chunk.NumVertices,
                .FirstVertex = chunk.Allocation.Start / static_cast<glm::u32>(sizeof(VertexData))
            };
            if (data.NumVertices == 0 || chunk.Allocation.Size == 0) continue;

            chunkDatas.push_back(data);
        }

        // create or update buffer
        if (!chunkDatas.empty()) {
            const glm::u32 requiredBufferSize = sizeof(chunkDatas[0]) * chunkDatas.size();
            if (m_chunkDatasBuffer.Count >= chunkDatas.size()) {
                // reuse buffer
                m_renderingManager.GetBufferManager().UpdateBuffer(m_chunkDatasBuffer, chunkDatas.data(), requiredBufferSize, 0);
            } else {
                FreeChunkDatasBuffer();
                m_chunkDatasBuffer = m_renderingManager.GetBufferManager().CreateStorageBuffer(chunkDatas.data(), requiredBufferSize, sizeof(chunkDatas[0]));
            }
        }

        Spire::info("chunk data size {}", chunkDatas.size());
    }

    void VoxelWorld::OnChunkEdited(Chunk &chunk) {
        // todo: what to do if now 0 allocation size/vertices?
        std::vector<VertexData> vertexData = chunk.GenerateMesh();

        BufferAllocator::Allocation oldAllocation = chunk.Allocation;
        chunk.Allocation = m_chunkVertexBufferAllocator.Allocate(vertexData.size() * sizeof(VertexData));

        // write the mesh into the vertex buffer
        m_chunkVertexBufferAllocator.Write(chunk.Allocation, vertexData.data(), vertexData.size() * sizeof(VertexData));

        // write the chunk data
        chunk.NumVertices = vertexData.size();

        WorldEditRequiredChanges changes = {false, false};
        if (oldAllocation.Size == 0) {
            CreateOrUpdateChunkDatasBuffer();
            changes.RecreatePipeline = true;
        } else {
            glm::u32 chunkIndex = 0;
            for (auto &[_,c] : m_chunks) {
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

    void VoxelWorld::Update() {
        m_chunkVertexBufferAllocator.Update();
    }

    std::unordered_map<glm::ivec3, Chunk>::iterator VoxelWorld::begin() {
        return m_chunks.begin();
    }

    std::unordered_map<glm::ivec3, Chunk>::iterator VoxelWorld::end() {
        return m_chunks.end();
    }

    void VoxelWorld::FreeChunkDatasBuffer() {
        if (m_chunkDatasBuffer.Buffer != VK_NULL_HANDLE) {
            m_renderingManager.GetBufferManager().DestroyBuffer(m_chunkDatasBuffer);
        }
    }
} // SpireVoxel
