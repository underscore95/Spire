#include "VoxelWorld.h"

namespace SpireVoxel {
    VoxelWorld::VoxelWorld(Spire::RenderingManager &renderingManager)
        : m_renderingManager(renderingManager),
          m_onPipelineRecreationRequiredDelegate() {

    }

    VoxelWorld::~VoxelWorld() {
        FreeChunkDatasBuffer();
    }

    Chunk &VoxelWorld::LoadChunk(glm::ivec3 chunkPosition) {
        auto it = m_chunks.find(chunkPosition);
        if (it != m_chunks.end()) return it->second;

        m_chunks.try_emplace(chunkPosition, m_renderingManager, chunkPosition);
        CreateChunkDatasBuffer();
        m_onPipelineRecreationRequiredDelegate.Broadcast();
        return m_chunks.find(chunkPosition)->second;
    }

    void VoxelWorld::LoadChunks(const std::vector<glm::ivec3> &chunkPositions) {
        bool loadedAnyChunks = false;

        for (auto chunkPosition : chunkPositions) {
            if (m_chunks.contains(chunkPosition)) continue;
            m_chunks.try_emplace(chunkPosition, m_renderingManager, chunkPosition);
            loadedAnyChunks = true;
        }

        if (loadedAnyChunks) {  CreateChunkDatasBuffer();
            m_onPipelineRecreationRequiredDelegate.Broadcast();
        }
    }

    void VoxelWorld::UnloadChunk(glm::ivec3 chunkPosition) {
        m_chunks.erase(chunkPosition);
        m_onPipelineRecreationRequiredDelegate.Broadcast();
        // todo handle chunk data
        // todo handle chunk data when changing voxels
    }

    DelegateSubscribers<> &VoxelWorld::GetOnPipelineRecreationRequiredSubscribers() {
        return m_onPipelineRecreationRequiredDelegate;
    }

    void VoxelWorld::CmdRender(VkCommandBuffer commandBuffer) {
        glm::u32 numVerticesToRender = 0;
        for (auto&[_,chunk] : m_chunks) {
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

        Spire::Descriptor descriptor = {
            .ResourceType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .Binding = SPIRE_VOXEL_SHADER_BINDINGS_CONSTANT_CHUNK_BINDING,
            .Stages = VK_SHADER_STAGE_VERTEX_BIT,
            .Resources = chunkVertexBufferPtrs,
#ifndef NDEBUG
            .DebugName = std::format("ChunkDescriptor ({} Vertex Datas)", chunkVertexBufferPtrs.size()),
#endif
        };
        chunkVertexBuffersLayout.push_back(descriptor);

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

    std::size_t VoxelWorld::NumLoadedChunks() const {
        return m_chunks.size();
    }

    void VoxelWorld::CreateChunkDatasBuffer() {
        if (m_chunks.empty()) return;

        // get chunk datas
        std::vector<ChunkData> chunkDatas;
        chunkDatas.reserve(m_chunks.size());
        for (const auto &[_, chunk] : m_chunks) {
            chunkDatas.push_back(chunk.GetChunkData());
        }

        const glm::u32 requiredBufferSize = sizeof(chunkDatas[0]) * chunkDatas.size();
        if (m_chunkDatasBuffer.Count >= m_chunks.size()) {
            // reuse buffer
            m_renderingManager.GetBufferManager().UpdateBuffer(m_chunkDatasBuffer, chunkDatas.data(), requiredBufferSize, 0);
        } else {
            FreeChunkDatasBuffer();
            m_chunkDatasBuffer = m_renderingManager.GetBufferManager().CreateStorageBuffer(chunkDatas.data(), requiredBufferSize, sizeof(chunkDatas[0]));
        }
    }

    void VoxelWorld::FreeChunkDatasBuffer() {
        if (m_chunkDatasBuffer.Buffer != VK_NULL_HANDLE) {
            m_renderingManager.GetBufferManager().DestroyBuffer(m_chunkDatasBuffer);
        }
    }
} // SpireVoxel
