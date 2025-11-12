#include "VoxelRenderer.h"

#include "Rendering/GameCamera.h"
#include "../Assets/Shaders/ShaderInfo.h"
#include "Chunk/VoxelWorld.h"

using namespace Spire;

constexpr glm::u32 NUM_CHUNKS = 1;

namespace SpireVoxel {
    VoxelRenderer::VoxelRenderer(Spire::Engine &engine)
        : m_engine(engine) {
        auto &rm = m_engine.GetRenderingManager();

        // Camera
        m_camera = std::make_unique<GameCamera>(m_engine);

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

        // Images
        std::vector<std::string> imagesToLoad = {"test.png"};

        assert(imagesToLoad.size() == SPIRE_SHADER_TEXTURE_COUNT);
        m_sceneImages = std::make_unique<SceneImages>(rm,ASSETS_DIRECTORY, imagesToLoad);

        // World
        m_world = std::make_unique<VoxelWorld>(rm);
        m_world->GetOnWorldEditSubscribers().AddCallback([this](VoxelWorld::WorldEditRequiredChanges changes) {
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

        // load 3x1x3 chunks around 0,0,0
        m_world->LoadChunks({
            {-1, 0, -1},
            {-1, 0, 0},
            {-1, 0, 1},
            {0, 0, -1},
            {0, 0, 0},
            {0, 0, 1},
            {1, 0, -1},
            {1, 0, 0},
            {1, 0, 1},
        });

        //m_world->LoadChunk({0,0,0});
        Chunk *chunk1 = m_world->GetLoadedChunk({0, 0, 0});
        assert(chunk1);
        //chunk1->VoxelData[SPIRE_VOXEL_POSITION_XYZ_TO_INDEX(0, 0, 0)] = 1;
        //m_world->OnChunkEdited(*chunk1);

        // Update world
        for (auto &[chunkPos, chunk] : *m_world) {
            chunk.VoxelData[SPIRE_VOXEL_POSITION_XYZ_TO_INDEX(0, 0, 0)] = 1;
            m_world->OnChunkEdited(chunk);
        }

        chunk1->VoxelData[SPIRE_VOXEL_POSITION_XYZ_TO_INDEX(0, 0, 0)] = 0;
        m_world->OnChunkEdited(*chunk1);
    }

    VoxelRenderer::~VoxelRenderer() {
        Cleanup();
    }

    void VoxelRenderer::Update() {
        static int i = 0;
        i++;
        m_oldDescriptors.Update();
        m_oldPipelines.Update();
        m_oldCommandBuffers.Update();
        m_world->Update();
        m_camera->Update();

        if (i == 10000) {
            Chunk *chunk1 = m_world->GetLoadedChunk({0, 0, 0});
            assert(chunk1);
            chunk1->VoxelData[SPIRE_VOXEL_POSITION_XYZ_TO_INDEX(0, 0, 0)] = 2;
            m_world->OnChunkEdited(*chunk1);
            info("Using allocation at {}",chunk1->Allocation.Start);
        }
    }

    VkCommandBuffer VoxelRenderer::Render(glm::u32 imageIndex) {
        RenderInfo renderInfo = {
            .ImageIndex = imageIndex
        };

        m_camera->Render(renderInfo);

        return (*m_commandBuffers)[imageIndex];
    }

    void VoxelRenderer::OnWindowResize() {
        SetupDescriptors();
        SetupGraphicsPipeline();
        CreateAndRecordCommandBuffers();
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
        Timer timer;

        auto &rm = m_engine.GetRenderingManager();

        // free any existing command buffers
        if (m_commandBuffers) {
            m_oldCommandBuffers.Push(std::move(m_commandBuffers), m_engine.GetRenderingManager().GetSwapchain().GetNumImages());
        }

        m_commandBuffers = std::make_unique<CommandBufferVector>(m_engine.GetRenderingManager(), m_engine.GetRenderingManager().GetSwapchain().GetNumImages());

        for (int i = 0; i < m_commandBuffers->Size(); ++i) {
            VkCommandBuffer commandBuffer = (*m_commandBuffers)[i];
            VkCommandBufferUsageFlags flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
            rm.GetCommandManager().BeginCommandBuffer(commandBuffer, flags);

            BeginRendering(commandBuffer, i);

            m_graphicsPipeline->CmdBindTo(commandBuffer);
            m_descriptorManager->CmdBind(commandBuffer, i, m_graphicsPipeline->GetLayout(), 0, 0);
            m_descriptorManager->CmdBind(commandBuffer, i, m_graphicsPipeline->GetLayout(), 1, 1);
            m_descriptorManager->CmdBind(commandBuffer, i, m_graphicsPipeline->GetLayout(), 4, 4);

            m_graphicsPipeline->CmdSetViewportToWindowSize(commandBuffer, m_engine.GetWindow().GetDimensions());

            m_world->CmdRender(commandBuffer);

            vkCmdEndRendering(commandBuffer);

            rm.GetCommandManager().EndCommandBuffer(commandBuffer);
        }

        info("Command buffers recorded in {} ms", timer.MillisSinceStart()); // 0.1 to 0.8 ms
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
        constantSet.push_back(m_sceneImages->GetDescriptor(SPIRE_SHADER_BINDINGS_MODEL_IMAGES_BINDING));

        // Camera
        perFrameSet.push_back(m_camera->GetDescriptor(SPIRE_SHADER_BINDINGS_CAMERA_UBO_BINDING));

        // Chunks
        m_world->PushDescriptors(constantSet, chunkSet);

        assert(layouts.Size() == SPIRE_SHADER_BINDINGS_CONSTANT_SET);
        layouts.Push(constantSet);
        assert(layouts.Size() == SPIRE_SHADER_BINDINGS_PER_FRAME_SET);
        layouts.Push(perFrameSet);
        assert(layouts.Size() == SPIRE_VOXEL_SHADER_BINDINGS_CONSTANT_CHUNK_SET);
        layouts.Push(chunkSet);

        // Create descriptor manager
        m_descriptorManager = std::make_unique<DescriptorManager>(m_engine.GetRenderingManager(), layouts);
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
            static_cast<glm::u32>(sizeof(PushConstants))
        );
    }

    void VoxelRenderer::Cleanup() {
        auto &rm = m_engine.GetRenderingManager();

        rm.GetQueue().WaitIdle();
        m_commandBuffers.reset();

        m_world.reset();
        m_camera.reset();

        m_sceneImages.reset();

        m_graphicsPipeline.reset();

        m_descriptorManager.reset();

        vkDestroyShaderModule(rm.GetDevice(), m_vertexShader, nullptr);
        vkDestroyShaderModule(rm.GetDevice(), m_fragmentShader, nullptr);
        info("Destroyed shaders");
    }
} // SpireVoxel
