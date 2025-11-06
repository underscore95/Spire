#include "VoxelRenderer.h"

#include "Rendering/GameCamera.h"
#include "../Assets/Shaders/ShaderInfo.h"

using namespace Spire;

struct ModelData {
    glm::mat4x4 ModelMatrix = glm::identity<glm::mat4x4>();
};

constexpr glm::u32 NUM_CHUNKS = 1;

namespace SpireVoxel {
    VoxelRenderer::VoxelRenderer(Spire::Engine &engine)
        : m_engine(engine) {
        auto &rm = m_engine.GetRenderingManager();

        // Camera
        m_camera = std::make_unique<GameCamera>(m_engine);

        // Shaders
        Timer shaderCompileTimer;
        shaderCompileTimer.Start();
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

        // Chunk
        m_chunk = std::make_unique<Chunk>(m_engine.GetRenderingManager(), glm::ivec3{0, 0, 0});
        m_chunk->SetVoxel({0, 0, 0}, 1);
        m_chunk->SetVoxelRect({2, 2, 2}, {3, 3, 3}, 2);

        // Descriptors, pipeline, command buffers
        PrepareForRendering();
    }

    VoxelRenderer::~VoxelRenderer() {
        Cleanup();
    }

    void VoxelRenderer::Update() {
        m_camera->Update();
    }

    VkCommandBuffer VoxelRenderer::Render(glm::u32 imageIndex) {
        RenderInfo renderInfo = {
            .ImageIndex = imageIndex
        };

        m_camera->Render(renderInfo);

        return m_commandBuffers[imageIndex];
    }

    void VoxelRenderer::OnWindowResize() {
        PrepareForRendering();
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
        if (!m_commandBuffers.empty()) {
            rm.GetCommandManager().FreeCommandBuffers(m_commandBuffers.size(), m_commandBuffers.data());
            m_commandBuffers.clear();
        }

        m_commandBuffers.resize(rm.GetSwapchain().GetNumImages());
        rm.GetCommandManager().CreateCommandBuffers(rm.GetSwapchain().GetNumImages(), m_commandBuffers.data());

        for (int i = 0; i < m_commandBuffers.size(); ++i) {
            VkCommandBuffer commandBuffer = m_commandBuffers[i];
            VkCommandBufferUsageFlags flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
            rm.GetCommandManager().BeginCommandBuffer(commandBuffer, flags);

            BeginRendering(commandBuffer, i);

            m_graphicsPipeline->CmdBindTo(commandBuffer);
            m_descriptorManager->CmdBind(commandBuffer, i, m_graphicsPipeline->GetLayout(), 0, 0);
            m_descriptorManager->CmdBind(commandBuffer, i, m_graphicsPipeline->GetLayout(), 1, 1);

            m_graphicsPipeline->CmdSetViewportToWindowSize(commandBuffer, m_engine.GetWindow().GetDimensions());

            if (m_chunk) {
                m_chunk->CmdBindIndexBuffer(commandBuffer);
                m_chunk->CmdRender(commandBuffer);
            }

            vkCmdEndRendering(commandBuffer);

            rm.GetCommandManager().EndCommandBuffer(commandBuffer);
        }

        info("Command buffers recorded");
    }

    void VoxelRenderer::SetupDescriptors() {
        DescriptorSetLayoutList layouts(m_engine.GetRenderingManager().GetSwapchain().GetNumImages()); {
            // Constant set
            assert(layouts.Size() == SPIRE_SHADER_BINDINGS_CONSTANT_SET);

            DescriptorSetLayout layout;

            // Chunks
            if (m_chunk) layout.push_back(m_chunk->GetDescriptor(SPIRE_SHADER_BINDINGS_VERTEX_SSBO_BINDING));

            // Images
            layout.push_back(m_sceneImages->GetDescriptor(SPIRE_SHADER_BINDINGS_MODEL_IMAGES_BINDING));

            layouts.Push(layout);
        } {
            assert(layouts.Size() == SPIRE_SHADER_BINDINGS_PER_FRAME_SET);
            // Per frame set
            PerImageDescriptorSetLayout layout;

            // Camera
            layout.push_back(m_camera->GetDescriptor(SPIRE_SHADER_BINDINGS_CAMERA_UBO_BINDING));

            layouts.Push(layout);
        }

        // Create descriptor manager
        m_descriptorManager = std::make_unique<DescriptorManager>(m_engine.GetRenderingManager(), layouts);
    }

    void VoxelRenderer::SetupGraphicsPipeline() {
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
        rm.GetCommandManager().FreeCommandBuffers(m_commandBuffers.size(), m_commandBuffers.data());
        info("Freed command buffers");

        m_chunk.reset();
        m_camera.reset();

        m_sceneImages.reset();

        m_graphicsPipeline.reset();

        m_descriptorManager.reset();

        vkDestroyShaderModule(rm.GetDevice(), m_vertexShader, nullptr);
        vkDestroyShaderModule(rm.GetDevice(), m_fragmentShader, nullptr);
        info("Destroyed shaders");
    }

    void VoxelRenderer::PrepareForRendering() {
        SetupDescriptors();
        SetupGraphicsPipeline();
        CreateAndRecordCommandBuffers();
    }
} // SpireVoxel
