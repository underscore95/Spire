#pragma once

#include "EngineIncludes.h"
#include "Chunk/Chunk.h"

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

        void RecordCommandBuffers() const;

        // returns paths to images to load
        std::vector<std::string> CreateModels();

        void SetupDescriptors();

        void SetupGraphicsPipeline();

        void Cleanup();

    private:
        Spire::Engine &m_engine;
        std::vector<VkCommandBuffer> m_commandBuffers;
        VkShaderModule m_vertexShader = VK_NULL_HANDLE;
        VkShaderModule m_fragmentShader = VK_NULL_HANDLE;
        std::unique_ptr<Spire::GraphicsPipeline> m_graphicsPipeline;
        std::unique_ptr<Spire::SceneModels> m_models;
        std::unique_ptr<Spire::SceneImages> m_sceneImages;
        std::unique_ptr<Spire::DescriptorManager> m_descriptorManager;
        std::unique_ptr<GameCamera> m_camera;
        std::unique_ptr<ObjectRenderer> m_chunkRenderer;
        Chunk m_chunk;
    };
} // SpireVoxel
