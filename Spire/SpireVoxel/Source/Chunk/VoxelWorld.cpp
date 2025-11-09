#include "VoxelWorld.h"

namespace SpireVoxel {
    VoxelWorld::VoxelWorld(Spire::RenderingManager &renderingManager)
        : m_renderingManager(renderingManager),
          m_onWorldEditedDelegate() {
        CreateOrUpdateChunkDatasBuffer();

        VertexData dummyVert = {};
        m_dummyVertexBuffer = renderingManager.GetBufferManager().CreateStorageBuffer(&dummyVert, sizeof(dummyVert), sizeof(dummyVert));

        ChunkData dummyChunkData = {};
        m_dummyChunkDataBuffer = renderingManager.GetBufferManager().CreateStorageBuffer(&dummyChunkData, sizeof(dummyChunkData), sizeof(dummyChunkData));
    }

    VoxelWorld::~VoxelWorld() {
        FreeChunkDatasBuffer();
        m_renderingManager.GetBufferManager().DestroyBuffer(m_dummyVertexBuffer);
        m_renderingManager.GetBufferManager().DestroyBuffer(m_dummyChunkDataBuffer);
    }

    Chunk &VoxelWorld::LoadChunk(glm::ivec3 chunkPosition) {
        auto it = m_chunks.find(chunkPosition);
        if (it != m_chunks.end()) return it->second;

        m_chunks.try_emplace(chunkPosition, m_renderingManager, chunkPosition, *this);
        CreateOrUpdateChunkDatasBuffer();
        m_onWorldEditedDelegate.Broadcast({true, false});
        return m_chunks.find(chunkPosition)->second;
    }

    void VoxelWorld::LoadChunks(const std::vector<glm::ivec3> &chunkPositions) {
        bool loadedAnyChunks = false;

        for (auto chunkPosition : chunkPositions) {
            if (m_chunks.contains(chunkPosition)) continue;
            m_chunks.try_emplace(chunkPosition, m_renderingManager, chunkPosition, *this);
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
            numVerticesToRender = std::max(numVerticesToRender, chunk.GetChunkData().NumVertices);
        }

        vkCmdDraw(commandBuffer, numVerticesToRender, m_chunks.size(), 0, 0);
    }

    void VoxelWorld::PushDescriptors(Spire::DescriptorSetLayout &constantDataLayout, Spire::DescriptorSetLayout &chunkVertexBuffersLayout) {
        std::vector<Spire::Descriptor::ResourcePtr> chunkVertexBufferPtrs;
        chunkVertexBufferPtrs.reserve(m_chunks.size());
        for (const auto &pair : m_chunks) {
            if (!pair.second.HasMesh()) continue;
            chunkVertexBufferPtrs.push_back({&pair.second.GetVertexBuffer()});
        }

        bool useDummyVertexBuffer = chunkVertexBufferPtrs.empty();
        if (useDummyVertexBuffer) {
            chunkVertexBufferPtrs.push_back({.Buffer = &m_dummyVertexBuffer});
        }

        for (auto &buf : chunkVertexBufferPtrs) {
            assert(buf .Buffer != VK_NULL_HANDLE);
        }

        Spire::Descriptor descriptor = {
            .ResourceType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .Binding = SPIRE_VOXEL_SHADER_BINDINGS_CONSTANT_CHUNK_BINDING,
            .Stages = VK_SHADER_STAGE_VERTEX_BIT,
            .Resources = chunkVertexBufferPtrs,
#ifndef NDEBUG
            .DebugName = useDummyVertexBuffer ? "Dummy Chunk Vertex Datas" : std::format("Chunk Vertex Datas ({} chunks)", chunkVertexBufferPtrs.size()),
#endif
        };
        chunkVertexBuffersLayout.push_back(descriptor);

        Spire::VulkanBuffer& chunkDataBuffer = m_chunkDatasBuffer.Buffer != VK_NULL_HANDLE ? m_chunkDatasBuffer : m_dummyChunkDataBuffer;
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
            ChunkData data = chunk.GetChunkData();
            if (data.NumVertices == 0) continue;

            chunkDatas.push_back(data);
        }

        if (!chunkDatas.empty()) {
            const glm::u32 requiredBufferSize = sizeof(chunkDatas[0]) * chunkDatas.size();
            if (m_chunkDatasBuffer.Count >= m_chunks.size()) {
                // reuse buffer
                m_renderingManager.GetBufferManager().UpdateBuffer(m_chunkDatasBuffer, chunkDatas.data(), requiredBufferSize, 0);
            } else {
                FreeChunkDatasBuffer();
                m_chunkDatasBuffer = m_renderingManager.GetBufferManager().CreateStorageBuffer(chunkDatas.data(), requiredBufferSize, sizeof(chunkDatas[0]));
            }
        }
    }

    void VoxelWorld::OnChunkEdited(Chunk &chunk, glm::u32 oldVertexCount) {
        ChunkData data = chunk.GetChunkData();
        if (data.NumVertices == oldVertexCount) return;
        if (oldVertexCount == 0) {
            CreateOrUpdateChunkDatasBuffer();
            m_onWorldEditedDelegate.Broadcast(WorldEditRequiredChanges{true, false});
            return;
        }

        glm::u32 index = 0;
        for (auto &[_,c] : m_chunks) {
            if (&c == &chunk) break;
            index++;
        }

        m_renderingManager.GetBufferManager().UpdateBuffer(m_chunkDatasBuffer, &data, sizeof(data), index * sizeof(data));
        m_onWorldEditedDelegate.Broadcast(oldVertexCount != 0 ? WorldEditRequiredChanges{false, true} : WorldEditRequiredChanges{true, false});
    }

    void VoxelWorld::FreeChunkDatasBuffer() {
        if (m_chunkDatasBuffer.Buffer != VK_NULL_HANDLE) {
            m_renderingManager.GetBufferManager().DestroyBuffer(m_chunkDatasBuffer);
        }
    }
} // SpireVoxel
