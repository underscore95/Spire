#include "ChunkMesherManager.h"

#include "Meshers/ChunkMesher.h"
#include "Meshers/GPUChunkMesher.h"

namespace SpireVoxel {
    ChunkMesherManager::ChunkMesherManager(Spire::RenderingManager &renderingManager) {
        m_meshers.push_back(std::make_unique<GPUChunkMesher>(renderingManager));
    }

    void ChunkMesherManager::Update() {
        for (std::size_t i = 0; i < m_queue.size(); i++) {
            std::shared_ptr<Chunk> chunk = m_queue[i].Chunk;

            if (m_currentlyMeshing.contains(chunk)) continue; // but maybe there has been changes since, so we will need to mesh again, but do it later

            Callback callback = std::move(m_queue[i].Callback);
            m_queue.erase(m_queue.begin() + i);
            i--;

            if (!chunk) {
                // chunk unloaded
                callback({}, MeshResult::ChunkUnloaded);
                continue;
            }

            m_currentlyMeshing.insert(chunk);

            callback(FindOptimalMesher().Mesh(*chunk).get(), MeshResult::Success);
        }
    }

    void ChunkMesherManager::Mesh(const std::shared_ptr<Chunk> &chunk, const Callback &callback) {
        for (const auto &queued : m_queue) {
            if (queued.Chunk == chunk) {
                callback({}, MeshResult::AlreadyInQueue);
                return;
            }
        }
        m_queue.push_back({chunk, callback});
    }

    ChunkMesher &ChunkMesherManager::FindOptimalMesher() const {
        return *m_meshers[0];
    }
} // SpireVoxel
