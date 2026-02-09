#pragma once
#include "../ChunkOrderControllers/IChunkOrderController.h"
#include "Providers/IProceduralGenerationProvider.h"

namespace SpireVoxel {
    class ProceduralGenerationManager {
    public:
        ProceduralGenerationManager(
            std::unique_ptr<IProceduralGenerationProvider> provider,
            std::unique_ptr<IChunkOrderController> controller,
            VoxelWorld &world,
            IVoxelCamera &camera);

    public:
        void Update();

        [[nodiscard]] glm::u32 NumChunksGeneratedThisFrame() const;

    private:
        std::unique_ptr<IProceduralGenerationProvider> m_provider;
        std::unique_ptr<IChunkOrderController> m_controller;
        VoxelWorld &m_world;
        IVoxelCamera &m_camera;
        glm::u32 m_numCPUThreads;
        glm::u32 m_numChunksGeneratedThisFrame = 0;
    };
} // SpireVoxel
