#pragma once

#include "EngineIncludes.h"
#include "Chunk/Chunk.h"
#include "Rendering/VoxelWorldRenderer.h"
#include "Utils/CommandBufferVector.h"
#include "Utils/FrameDeleter.h"

namespace SpireVoxel {
    const char *GetAssetsDirectory();

    class IVoxelCamera;
    class VoxelImageManager;
    class VoxelTypeRegistry;
    class VoxelWorld;
    class ObjectRenderer;

    class VoxelRenderer {
    public:
        explicit VoxelRenderer(Spire::Engine &engine, IVoxelCamera &camera, std::unique_ptr<VoxelWorld> world,
                               const std::function<void(VoxelTypeRegistry &)> &registerVoxelTypesFunc);

        ~VoxelRenderer();

    public:
        // Call this once per frame
        void Update();

        // Call this once per frame
        [[nodiscard]] VkCommandBuffer Render(glm::u32 imageIndex) const;

        void OnWindowResize();

        [[nodiscard]] VoxelWorld &GetWorld() const;

        [[nodiscard]] glm::u64 GetCurrentFrame() const;

    private:
        void BeginRendering(VkCommandBuffer commandBuffer, glm::u32 imageIndex) const;

        void CreateAndRecordCommandBuffers();

        void SetupDescriptors();

        void SetupGraphicsPipeline();

        void Cleanup();

    public:
        static constexpr bool RENDER_WIREFRAMES = false;

    private:
        Spire::Engine &m_engine;
        VkShaderModule m_vertexShader = VK_NULL_HANDLE;
        VkShaderModule m_fragmentShader = VK_NULL_HANDLE;
        std::unique_ptr<VoxelTypeRegistry> m_voxelTypeRegistry;
        std::unique_ptr<VoxelImageManager> m_voxelImageManager;
        std::unique_ptr<Spire::GraphicsPipeline> m_graphicsPipeline;
        std::unique_ptr<Spire::CommandBufferVector> m_commandBuffers;
        std::unique_ptr<Spire::DescriptorManager> m_descriptorManager;
        IVoxelCamera &m_camera;
        std::unique_ptr<VoxelWorld> m_world;
        // need to keep old command buffers around until previous frames are no longer using them
        Spire::FrameDeleter<std::unique_ptr<Spire::CommandBufferVector> > m_oldCommandBuffers;
        glm::u64 m_currentFrame = 0;
        int m_worldEditCallback;
    };
} // SpireVoxel
