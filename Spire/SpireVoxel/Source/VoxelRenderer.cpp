// ReSharper disable CppDFAUnreachableCode
#include "VoxelRenderer.h"

#include "../Assets/Shaders/ShaderInfo.h"
#include "Chunk/VoxelWorld.h"
#include "Edits/BasicVoxelEdit.h"
#include "Rendering/VoxelWorldRenderer.h"
#include "Serialisation/VoxelSerializer.h"
#include "Types/VoxelImageManager.h"
#include "Types/VoxelType.h"
#include "Types/VoxelTypeRegistry.h"
#include "Utils/IVoxelCamera.h"

using namespace Spire;

namespace SpireVoxel {
    VoxelRenderer::VoxelRenderer(Engine &engine, IVoxelCamera &camera)
        : m_engine(engine), m_camera(camera) {
        auto &rm = m_engine.GetRenderingManager();

        // Shaders
        Timer shaderCompileTimer;
        shaderCompileTimer.Restart();
        ShaderCompiler compiler(rm.GetDevice());
        info("Created shader compiler");
        compiler.CreateShaderModuleAsync(&m_vertexShader, std::format("{}/Shaders/test.vert", ASSETS_DIRECTORY));
        compiler.CreateShaderModuleAsync(&m_fragmentShader, std::format("{}/Shaders/test.frag", ASSETS_DIRECTORY));
        compiler.Await();
        assert(m_vertexShader != VK_NULL_HANDLE);
        assert(m_fragmentShader != VK_NULL_HANDLE);
        info("Created shaders in {} ms", 1000.0f * shaderCompileTimer.SecondsSinceStart());

        // Voxel types
        RegisterVoxelTypes();

        // World
        m_world = std::make_unique<VoxelWorld>(rm);
        m_worldEditCallback = m_world->GetRenderer().GetOnWorldEditSubscribers().AddCallback([this](VoxelWorldRenderer::WorldEditRequiredChanges changes) {
            if (changes.RecreatePipeline) {
                // Takes 1.6ms on my PC
                SetupDescriptors();
                SetupGraphicsPipeline();
                CreateAndRecordCommandBuffers();
            } else if (changes.RecreateOnlyCommandBuffers) {
                CreateAndRecordCommandBuffers();
            } else
                assert(false);
        });

        if (IS_PROFILING) {
            VoxelSerializer::ClearAndDeserialize(*m_world, std::filesystem::path("Worlds") / WORLD_NAME);
            info("Loaded {} chunks from world file {}", m_world->NumLoadedChunks(), WORLD_NAME);
        } else VoxelSerializer::ClearAndDeserialize(*m_world, std::filesystem::path("Worlds") / "Test3");


        m_timeSinceBeginProfiling.Restart();
    }

    VoxelRenderer::~VoxelRenderer() {
        Cleanup();
    }

    void VoxelRenderer::Update() {
        m_currentFrame++;
        m_oldDescriptors.Update();
        m_oldPipelines.Update();
        m_oldCommandBuffers.Update();

        HandleProfiling();
    }

    VkCommandBuffer VoxelRenderer::Render(glm::u32 imageIndex) const {
        m_world->GetRenderer().Render(imageIndex);

        RenderInfo renderInfo = {
            .ImageIndex = imageIndex
        };

        m_camera.Render(renderInfo);

        return (*m_commandBuffers)[imageIndex];
    }

    void VoxelRenderer::OnWindowResize() {
        SetupDescriptors();
        SetupGraphicsPipeline();
        CreateAndRecordCommandBuffers();
    }

    VoxelWorld &VoxelRenderer::GetWorld() const {
        return *m_world;
    }

    void VoxelRenderer::BeginRendering(VkCommandBuffer commandBuffer, glm::u32 imageIndex) const {
        auto &rm = m_engine.GetRenderingManager();

        VkClearValue clearColor = {
            .color = {0.0f, 0.0f, 0.0f, 1.0f},
        };

        VkClearValue clearDepthValue = {
            .depthStencil = {.depth = 1.0f, .stencil = 0}
        };

        rm.GetRenderer().BeginDynamicRendering(commandBuffer, imageIndex, &clearColor, &clearDepthValue);
    }

    void VoxelRenderer::CreateAndRecordCommandBuffers() {
        auto &rm = m_engine.GetRenderingManager();

        // free any existing command buffers
        if (m_commandBuffers) {
            m_oldCommandBuffers.Push(std::move(m_commandBuffers), m_engine.GetRenderingManager().GetSwapchain().GetNumImages());
        }

        m_commandBuffers = std::make_unique<CommandBufferVector>(m_engine.GetRenderingManager(), m_engine.GetRenderingManager().GetSwapchain().GetNumImages());

        for (int swapchainImage = 0; swapchainImage < m_commandBuffers->Size(); ++swapchainImage) {
            VkCommandBuffer commandBuffer = (*m_commandBuffers)[swapchainImage];
            VkCommandBufferUsageFlags flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
            rm.GetCommandManager().BeginCommandBuffer(commandBuffer, flags);

            BeginRendering(commandBuffer, swapchainImage);

            m_graphicsPipeline->CmdBindTo(commandBuffer);
            m_descriptorManager->CmdBind(commandBuffer, swapchainImage, m_graphicsPipeline->GetLayout(), 0, 0);
            m_descriptorManager->CmdBind(commandBuffer, swapchainImage, m_graphicsPipeline->GetLayout(), 1, 1);
            m_descriptorManager->CmdBind(commandBuffer, swapchainImage, m_graphicsPipeline->GetLayout(), 4, 4);

            m_graphicsPipeline->CmdSetViewportToWindowSize(commandBuffer, m_engine.GetWindow().GetDimensions());

            m_world->GetRenderer().CmdRender(swapchainImage, commandBuffer);

            vkCmdEndRendering(commandBuffer);

            rm.GetCommandManager().EndCommandBuffer(commandBuffer);
        }

        //  info("Command buffers recorded in {} ms", timer.MillisSinceStart()); // 0.1 to 0.8 ms
    }

    void VoxelRenderer::SetupDescriptors() {
        if (m_descriptorManager) {
            m_oldDescriptors.Push(std::move(m_descriptorManager), m_engine.GetRenderingManager().GetSwapchain().GetNumImages());
        }

        DescriptorSetLayoutList layouts(m_engine.GetRenderingManager().GetSwapchain().GetNumImages());

        DescriptorSetLayout constantSet;
        PerImageDescriptorSetLayout perFrameSet;
        DescriptorSetLayout chunkSet;

        // Images
        constantSet.push_back(m_voxelImageManager->GetDescriptor());

        // Camera
        perFrameSet.push_back(m_camera.GetDescriptor(SPIRE_SHADER_BINDINGS_CAMERA_UBO_BINDING));

        // Chunks
        m_world->GetRenderer().PushDescriptors(perFrameSet, chunkSet);

        // Voxel types
        constantSet.push_back(m_voxelTypeRegistry->GetVoxelTypesBufferDescriptor(SPIRE_VOXEL_SHADER_BINDINGS_VOXEL_TYPE_UBO_BINDING));

        assert(layouts.Size() == SPIRE_SHADER_BINDINGS_CONSTANT_SET);
        layouts.Push(constantSet);
        assert(layouts.Size() == SPIRE_SHADER_BINDINGS_PER_FRAME_SET);
        layouts.Push(perFrameSet);
        assert(layouts.Size() == SPIRE_VOXEL_SHADER_BINDINGS_CONSTANT_CHUNK_SET);
        layouts.Push(chunkSet);

        // Create descriptor manager
        m_descriptorManager = std::make_unique<DescriptorManager>(m_engine.GetRenderingManager(), layouts, VK_PIPELINE_BIND_POINT_GRAPHICS);
    }

    void VoxelRenderer::SetupGraphicsPipeline() {
        if (m_graphicsPipeline) {
            m_oldPipelines.Push(std::move(m_graphicsPipeline), m_engine.GetRenderingManager().GetSwapchain().GetNumImages());
        }

        auto &rm = m_engine.GetRenderingManager();

        m_graphicsPipeline = std::make_unique<GraphicsPipeline>(
            rm.GetDevice(),
            m_vertexShader,
            m_fragmentShader,
            *m_descriptorManager,
            rm.GetSwapchain().GetSurfaceFormat().format,
            rm.GetPhysicalDevice().DepthFormat,
            rm,
            static_cast<glm::u32>(sizeof(PushConstants)),
            RENDER_WIREFRAMES
        );
    }

    void VoxelRenderer::Cleanup() {
        auto &rm = m_engine.GetRenderingManager();

        rm.GetQueue().WaitIdle();
        m_commandBuffers.reset();

        m_world->GetRenderer().GetOnWorldEditSubscribers().RemoveCallback(m_worldEditCallback);
        m_world.reset();

        m_voxelImageManager.reset();
        m_voxelTypeRegistry.reset();

        m_graphicsPipeline.reset();

        m_descriptorManager.reset();

        vkDestroyShaderModule(rm.GetDevice(), m_vertexShader, nullptr);
        vkDestroyShaderModule(rm.GetDevice(), m_fragmentShader, nullptr);
        info("Destroyed shaders");
    }

    void VoxelRenderer::RegisterVoxelTypes() {
        m_voxelTypeRegistry = std::make_unique<VoxelTypeRegistry>(m_engine.GetRenderingManager());
        m_voxelImageManager = std::make_unique<VoxelImageManager>(m_engine.GetRenderingManager(), *m_voxelTypeRegistry, SPIRE_VOXEL_SHADER_BINDINGS_IMAGES_BINDING);

        m_voxelTypeRegistry->RegisterTypes(std::vector<VoxelType>{
            {
                1, {
                    std::string(ASSETS_DIRECTORY) + "/grass_top.png",
                    std::string(ASSETS_DIRECTORY) + "/dirt.png",
                    std::string(ASSETS_DIRECTORY) + "/grass_side.png"
                },
                SPIRE_VOXEL_LAYOUT_TOP_DIFFERENT_BOTTOM_DIFFERENT
            },
            {2, {std::string(ASSETS_DIRECTORY) + "/dirt.png"}, SPIRE_VOXEL_LAYOUT_ALL_SAME},
        });
    }

    void VoxelRenderer::HandleProfiling() {
        if constexpr (!IS_PROFILING) return;
        if (m_profileStrategyIndex >= PROFILE_STRATEGIES.size()) return;
        if (m_currentFrame == 1) {
            info("Profiling... (world: {})", WORLD_NAME);
#ifndef NDEBUG
            warn("Profiling in debug mode, results will be inaccurate!");
#endif
        }

        const ProfileStrategy &profileStrategy = PROFILE_STRATEGIES[m_profileStrategyIndex];

        glm::u32 profileEndFrame = profileStrategy.FramesToProfile;
        for (std::size_t i = 0; i < m_profileStrategyIndex; i++) {
            profileEndFrame += PROFILE_STRATEGIES[i].FramesToProfile;
        }

        if (profileStrategy.Dynamic == ProfileStrategy::DYNAMIC) {
            for (auto &[_,chunk] : *m_world) {
                chunk->VoxelData[SPIRE_VOXEL_POSITION_XYZ_TO_INDEX(1, 1, 1)] = chunk->VoxelData[SPIRE_VOXEL_POSITION_XYZ_TO_INDEX(1, 1, 1)] == 0 ? 1 : 0;
                m_world->GetRenderer().NotifyChunkEdited(*chunk);
            }
        } else {
            assert(profileStrategy.Dynamic == ProfileStrategy::STATIC);
        }

        if (m_currentFrame == profileEndFrame) {
            m_profileStrategyIndex++;
            m_profileJson += std::format(
                R"({{"time_ms": {}, "frames": {}, "chunks": {}, "world": "{}", "dynamic_state": "{}", "chunk_gpu_memory": {}, "chunk_cpu_memory": {}, "window_width": {}, "window_height": {}}}, )",
                m_timeSinceBeginProfiling.MillisSinceStart(),
                profileStrategy.FramesToProfile,
                m_world->NumLoadedChunks(),
                WORLD_NAME,
                profileStrategy.Dynamic,
                m_world->CalculateGPUMemoryUsageForChunks(),
                m_world->CalculateCPUMemoryUsageForChunks(),
                m_engine.GetWindow().GetDimensions().x,
                m_engine.GetWindow().GetDimensions().y
            );
            m_timeSinceBeginProfiling.Restart();

            if (m_profileStrategyIndex >= PROFILE_STRATEGIES.size()) {
                info("Finished profiling! {}", m_profileJson);
            } else {
                info("Using profile strategy index {}...", m_profileStrategyIndex);
            }
        }
    }
} // SpireVoxel
