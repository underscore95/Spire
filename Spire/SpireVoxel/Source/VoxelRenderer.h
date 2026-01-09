#pragma once

#include "EngineIncludes.h"
#include "Chunk/Chunk.h"
#include "Rendering/VoxelWorldRenderer.h"
#include "Utils/CommandBufferVector.h"
#include "Utils/FrameDeleter.h"

namespace SpireVoxel {
    class IVoxelCamera;
}

namespace SpireVoxel {
    class VoxelImageManager;
}

namespace SpireVoxel {
    class VoxelTypeRegistry;
}

namespace SpireVoxel {
    class VoxelWorld;
}

namespace SpireVoxel {
    class ObjectRenderer;

    class VoxelRenderer {
    public:
        explicit VoxelRenderer(Spire::Engine &engine, IVoxelCamera &camera);

        ~VoxelRenderer();

    public:
        void Update();

        [[nodiscard]] VkCommandBuffer Render(glm::u32 imageIndex) const;

        void OnWindowResize();

        [[nodiscard]] VoxelWorld &GetWorld() const;

    private:
        void BeginRendering(VkCommandBuffer commandBuffer, glm::u32 imageIndex) const;

        void CreateAndRecordCommandBuffers();

        void SetupDescriptors();

        void SetupGraphicsPipeline();

        void Cleanup();

        void RegisterVoxelTypes();

        void HandleProfiling();

    public:
#ifndef NDEBUG
        static constexpr bool IS_PROFILING = false; // debug
#else
        static constexpr bool IS_PROFILING = true; // release
#endif
        static constexpr bool RENDER_WIREFRAMES = false;

    private:
        struct ProfileStrategy {
            typedef const char *DynamicState;
            static constexpr DynamicState STATIC = "static"; // no changes to world
            static constexpr DynamicState DYNAMIC = "dynamic"; // change 1 voxel in every chunk every frame

            DynamicState Dynamic;
            glm::u64 FramesToProfile;
        };

        static constexpr const char *PROFILE_WORLD_NAME = "Test6";

        static constexpr ProfileStrategy PROFILE_STATIC = {ProfileStrategy::STATIC, 10000};
        static constexpr ProfileStrategy PROFILE_STATIC_1000 = {ProfileStrategy::STATIC, 1000};
        static constexpr ProfileStrategy PROFILE_DYNAMIC_10 = {ProfileStrategy::DYNAMIC, 10};
        static constexpr ProfileStrategy PROFILE_DYNAMIC = {ProfileStrategy::DYNAMIC, 100};
        //  static constexpr std::array<ProfileStrategy, 2> PROFILE_STRATEGIES = {PROFILE_STATIC, PROFILE_DYNAMIC};
        static constexpr std::array<ProfileStrategy, 2> PROFILE_STRATEGIES = {PROFILE_STATIC_1000, PROFILE_DYNAMIC_10};

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
        Spire::FrameDeleter<std::unique_ptr<Spire::CommandBufferVector> > m_oldCommandBuffers;
        Spire::Timer m_timeSinceBeginProfiling;
        glm::u64 m_currentFrame = 0;
        std::size_t m_profileStrategyIndex = 0;
        std::string m_profileJson;
        int m_worldEditCallback;
    };
} // SpireVoxel
