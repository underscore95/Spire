#include "ProceduralGenerationManager.h"

#include "Chunk/VoxelWorld.h"
#include "Rendering/VoxelWorldRenderer.h"
#include "Utils/ThreadPool.h"

namespace SpireVoxel {
    static constexpr bool LOG = true;

    ProceduralGenerationManager::ProceduralGenerationManager(
        std::unique_ptr<IProceduralGenerationProvider> provider,
        std::unique_ptr<IProceduralGenerationController> controller,
        VoxelWorld &world,
        IVoxelCamera &camera)
        : m_provider(std::move(provider)),
          m_controller(std::move(controller)),
          m_world(world),
          m_camera(camera) {
        m_numCPUThreads = std::thread::hardware_concurrency();
    }

    void ProceduralGenerationManager::Update() const {
        glm::u32 numToGenerate = std::max(0, static_cast<glm::i32>(m_numCPUThreads) - static_cast<glm::i32>(m_world.GetRenderer().NumEditedChunks()));

        std::vector<glm::ivec3> coordsToLoad = m_controller->GetChunkCoordsToLoad(m_world, m_camera, numToGenerate);
        m_world.LoadChunks(coordsToLoad);

        std::vector<Chunk *> chunksToGenerate;
        for (glm::ivec3 coord : coordsToLoad) {
            Chunk *chunk = m_world.GetLoadedChunk(coord);
            if (chunk) {
                chunksToGenerate.push_back(chunk);
            }
        }

        Spire::Timer timer;
        Spire::ThreadPool::Instance().submit_loop(0, chunksToGenerate.size(), [this, &chunksToGenerate](glm::u32 i) {
            if (chunksToGenerate[i] == nullptr) {
                Spire::error("Failed to generate a chunk because it was nullptr");
                return;
            }

            m_provider->GenerateChunk(m_world, *chunksToGenerate[i]);
        }).get();

        if (LOG && !chunksToGenerate.empty()) {
            Spire::info("[ProceduralGenerationManager] Generated {} chunks in {} ms", chunksToGenerate.size(), timer.MillisSinceStart());
        }
    }
} // SpireVoxel
