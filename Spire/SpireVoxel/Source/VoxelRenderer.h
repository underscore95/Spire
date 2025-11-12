#pragma once

#include "EngineIncludes.h"
#include "Chunk/Chunk.h"
#include "Utils/CommandBufferVector.h"
#include "Utils/FrameDeleter.h"

namespace SpireVoxel {
    class VoxelWorld;
}

namespace SpireVoxel {
    class ObjectRenderer;
    class GameCamera;

    class VoxelRenderer {
    public:
        explicit VoxelRenderer(Spire::Engine &engine);

        ~VoxelRenderer();

    public:
        void Update();

        VkCommandBuffer Render(glm::u32 imageIndex);

        void OnWindowResize();

    private:
        void BeginRendering(VkCommandBuffer commandBuffer, glm::u32 imageIndex) const;

        void CreateAndRecordCommandBuffers();

        void SetupDescriptors();

        void SetupGraphicsPipeline();

        void Cleanup();

    private:
        Spire::Engine &m_engine;
        VkShaderModule m_vertexShader = VK_NULL_HANDLE;
        VkShaderModule m_fragmentShader = VK_NULL_HANDLE;
        std::unique_ptr<Spire::SceneImages> m_sceneImages;
        std::unique_ptr<Spire::GraphicsPipeline> m_graphicsPipeline;
        std::unique_ptr<Spire::CommandBufferVector> m_commandBuffers;
        std::unique_ptr<Spire::DescriptorManager> m_descriptorManager;
        std::unique_ptr<GameCamera> m_camera;
        std::unique_ptr<VoxelWorld> m_world;
        Spire::FrameDeleter<std::unique_ptr<Spire::GraphicsPipeline> > m_oldPipelines;
        Spire::FrameDeleter<std::unique_ptr<Spire::DescriptorManager> > m_oldDescriptors;
        Spire::FrameDeleter<std::unique_ptr<Spire::CommandBufferVector> > m_oldCommandBuffers;
    };
} // SpireVoxel
