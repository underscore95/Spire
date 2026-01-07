#include "ChunkMesherManager.h"

#include "Meshers/ChunkMesher.h"
#include "Meshers/GPUChunkMesher.h"

namespace SpireVoxel {
    ChunkMesherManager::ChunkMesherManager(Spire::RenderingManager &renderingManager) {
        m_meshers.push_back(std::make_unique<GPUChunkMesher>(renderingManager));

        for (auto &mesher : m_meshers) {
            mesher->Init();
        }
    }

    void ChunkMesherManager::Update() {
        // Begin meshing
        for (std::size_t i = 0; i < m_queue.size(); i++) {
            std::shared_ptr<Chunk> chunk = m_queue[i].Chunk;

            if (m_currentlyMeshing.contains(chunk)) continue; // but maybe there has been changes since, so we will need to mesh again, but do it later

            Callback callback = std::move(m_queue[i].Callback);
            m_queue[i] = m_queue.back();
            m_queue.pop_back();
            i--;

            if (!chunk) {
                // chunk unloaded
                callback({}, MeshResult::ChunkUnloaded);
                continue;
            }

            // Find mesher
            for (auto &mesher : m_meshers) {
                if (mesher->GetLoad() < 0) continue;
                std::optional threadIdOpt = mesher->LockThreadId();
                if (!threadIdOpt) continue;
                std::future future = std::async(std::launch::async, [this, chunk,threadIdOpt, &mesher] { return mesher->Mesh(chunk, threadIdOpt.value()); });
                m_currentlyMeshing[chunk] = {std::move(callback), std::move(future)};
                break;
            }
        }

        // Wait until meshing complete
        for (auto &[_,processingChunk] : m_currentlyMeshing) {
            processingChunk.Callback(processingChunk.Future.get(), MeshResult::Success);
        }
        Spire::info("Meshed {} this frame", m_currentlyMeshing.size());
        m_currentlyMeshing.clear();
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
} // SpireVoxel
