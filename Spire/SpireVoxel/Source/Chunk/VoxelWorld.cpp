#include "VoxelWorld.h"

#include "Rendering/VoxelWorldRenderer.h"

namespace SpireVoxel {
    VoxelWorld::VoxelWorld(Spire::RenderingManager &renderingManager) {
        m_renderer = std::make_unique<VoxelWorldRenderer>(*this, renderingManager);
    }

    Chunk &VoxelWorld::LoadChunk(glm::ivec3 chunkPosition) {
        auto it = m_chunks.find(chunkPosition);
        if (it != m_chunks.end()) return it->second;

        m_chunks.try_emplace(chunkPosition, chunkPosition);
        m_renderer->NotifyChunkLoadedOrUnloaded();
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
            m_renderer->NotifyChunkLoadedOrUnloaded();
        }
    }

    void VoxelWorld::UnloadChunks(const std::vector<glm::ivec3> &chunkPositions) {
        bool unloadedAnyChunks = false;
        for (auto chunkPosition : chunkPositions) {
            if (!m_chunks.contains(chunkPosition)) continue;
            m_chunks.erase(chunkPosition);
            unloadedAnyChunks = true;
        }

        if (unloadedAnyChunks) {
            m_renderer->NotifyChunkLoadedOrUnloaded();
        }
    }

    Chunk *VoxelWorld::GetLoadedChunk(glm::ivec3 chunkPosition) {
        auto it = m_chunks.find(chunkPosition);
        return it != m_chunks.end() ? &it->second : nullptr;
    }

    std::size_t VoxelWorld::NumLoadedChunks() const {
        return m_chunks.size();
    }

    std::unordered_map<glm::ivec3, Chunk>::iterator VoxelWorld::begin() {
        return m_chunks.begin();
    }

    std::unordered_map<glm::ivec3, Chunk>::iterator VoxelWorld::end() {
        return m_chunks.end();
    }

    std::uint32_t VoxelWorld::GetNumLoadedChunks() const {
        return m_chunks.size();
    }

    void VoxelWorld::UnloadAllChunks() {
        std::vector<glm::ivec3> loadedChunks;
        loadedChunks.reserve(GetNumLoadedChunks());
        for (auto &[chunkPos, _] : m_chunks) {
            loadedChunks.push_back(chunkPos);
        }

        UnloadChunks(loadedChunks);
    }

    VoxelWorldRenderer &VoxelWorld::GetRenderer() const {
        return *m_renderer;
    }
} // SpireVoxel
