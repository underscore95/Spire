#include "VoxelWorld.h"

namespace SpireVoxel {
    VoxelWorld::VoxelWorld(Spire::RenderingManager &renderingManager)
        : m_renderingManager(renderingManager),
          m_onChunkLoadOrUnloadDelegate() {
    }

    Chunk &VoxelWorld::LoadChunk(glm::ivec3 chunkPosition) {
        auto it = m_chunks.find(chunkPosition);
        if (it != m_chunks.end()) return it->second;

        m_chunks.try_emplace(chunkPosition, m_renderingManager, chunkPosition);
        m_onChunkLoadOrUnloadDelegate.Broadcast();
        return m_chunks.find(chunkPosition)->second;
    }

    void VoxelWorld::LoadChunks(const std::vector<glm::ivec3> &chunkPositions) {
        bool loadedAnyChunks = false;

        for (auto chunkPosition : chunkPositions) {
            if (m_chunks.contains(chunkPosition)) continue;
            m_chunks.try_emplace(chunkPosition, m_renderingManager, chunkPosition);
            loadedAnyChunks = true;
        }

        if (loadedAnyChunks) {
            m_onChunkLoadOrUnloadDelegate.Broadcast();
        }
    }

    void VoxelWorld::UnloadChunk(glm::ivec3 chunkPosition) {
        m_chunks.erase(chunkPosition);
        m_onChunkLoadOrUnloadDelegate.Broadcast();
    }

    DelegateSubscribers<> &VoxelWorld::GetOnChunkLoadOrUnloadSubscribers() {
        return m_onChunkLoadOrUnloadDelegate;
    }

    void VoxelWorld::CmdRender(VkCommandBuffer commandBuffer) {
        for (auto &[_,chunk] : m_chunks) {
            chunk.CmdBindIndexBuffer(commandBuffer);
            chunk.CmdRender(commandBuffer);
        }
    }

    void VoxelWorld::PushDescriptors(Spire::DescriptorSetLayout &layout, glm::u32 binding) {
        std::vector<Spire::Descriptor::ResourcePtr> chunkVertexBufferPtrs;
        chunkVertexBufferPtrs.reserve(m_chunks.size());
        for (const auto &pair : m_chunks) {
            if (!pair.second.HasMesh()) continue;
            chunkVertexBufferPtrs.push_back({&pair.second.GetVertexBuffer()});
        }

        Spire::Descriptor descriptor = {
            .ResourceType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .Binding = binding,
            .Stages = VK_SHADER_STAGE_VERTEX_BIT,
            .Resources = chunkVertexBufferPtrs,
#ifndef NDEBUG
            .DebugName = std::format("ChunkDescriptor ({} Vertex Datas)", chunkVertexBufferPtrs.size()),
#endif
        };

        layout.push_back(descriptor);
    }

    std::size_t VoxelWorld::NumLoadedChunks() const {
        return m_chunks.size();
    }
} // SpireVoxel
