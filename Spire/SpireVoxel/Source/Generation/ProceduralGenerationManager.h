#pragma once
#include "Controllers/IProceduralGenerationController.h"
#include "Providers/IProceduralGenerationProvider.h"

namespace SpireVoxel {
    class ProceduralGenerationManager {
    public:
        ProceduralGenerationManager(
            std::unique_ptr<IProceduralGenerationProvider> provider,
            std::unique_ptr<IProceduralGenerationController> controller,
            VoxelWorld &world,
            IVoxelCamera &camera);

    public:
        void Update() const;

    private:
        std::unique_ptr<IProceduralGenerationProvider> m_provider;
        std::unique_ptr<IProceduralGenerationController> m_controller;
        VoxelWorld &m_world;
        IVoxelCamera &m_camera;
        glm::u32 m_numCPUThreads;
    };
} // SpireVoxel
