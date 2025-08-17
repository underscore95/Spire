#include "GameApplication.h"
#include <libassert/assert.hpp>
#include <spdlog/spdlog.h>
#include <glm/gtc/matrix_transform.hpp>
#include "GameCamera.h"
#include "../Assets/Shaders/ShaderBindings.h"

using namespace Spire;

GameApplication::GameApplication()
{
    // don't do anything here!
}

void GameApplication::Start(Engine& engine)
{
    m_engine = &engine;

    auto& rm = m_engine->GetRenderingManager();

    // Camera
    m_camera = std::make_unique<GameCamera>(*m_engine);

    // Shaders
    Timer shaderCompileTimer;
    shaderCompileTimer.Start();
    ShaderCompiler compiler(rm.GetDevice());
    spdlog::info("Created shader compiler");
    compiler.CreateShaderModuleAsync(&m_vertexShader, std::format("{}/Shaders/test.vert", ASSETS_DIRECTORY));
    compiler.CreateShaderModuleAsync(&m_fragmentShader, std::format("{}/Shaders/test.frag", ASSETS_DIRECTORY));
    compiler.Await();
    DEBUG_ASSERT(m_vertexShader != VK_NULL_HANDLE);
    DEBUG_ASSERT(m_fragmentShader != VK_NULL_HANDLE);
    spdlog::info("Created shaders in {} ms", 1000.0f * shaderCompileTimer.SecondsSinceStart());

    // Shader data
    std::vector<std::string> imagesToLoad = CreateModels();

    // Images
    while (imagesToLoad.size() < 2) imagesToLoad.push_back("test.png"); // shader forces 2 images to be bound so hack
    m_sceneImages = std::make_unique<SceneImages>(rm, imagesToLoad);

    // Descriptors
    SetupDescriptors();

    // Pipeline
    SetupGraphicsPipeline();

    // Command buffers
    m_commandBuffers.resize(rm.GetSwapchain().GetNumImages());
    rm.GetCommandManager().CreateCommandBuffers(rm.GetSwapchain().GetNumImages(), m_commandBuffers.data());
    RecordCommandBuffers();
}

GameApplication::~GameApplication()
{
    Cleanup();
}

void GameApplication::Cleanup()
{
    auto& rm = m_engine->GetRenderingManager();

    rm.GetQueue().WaitIdle();
    rm.GetCommandManager().FreeCommandBuffers(m_commandBuffers.size(), m_commandBuffers.data());
    spdlog::info("Freed command buffers");

    m_models.reset();
    m_camera.reset();

    m_sceneImages.reset();

    m_graphicsPipeline.reset();

    m_descriptorManager.reset();

    vkDestroyShaderModule(rm.GetDevice(), m_vertexShader, nullptr);
    vkDestroyShaderModule(rm.GetDevice(), m_fragmentShader, nullptr);
    spdlog::info("Destroyed shaders");
}

void GameApplication::Update()
{
    m_camera->Update();
}

void GameApplication::Render()
{
    auto& rm = m_engine->GetRenderingManager();

    glm::u32 imageIndex = rm.GetQueue().AcquireNextImage();
    if (imageIndex == rm.GetQueue().INVALID_IMAGE_INDEX) return;

    RenderInfo renderInfo = {
        .ImageIndex = imageIndex
    };

    m_camera->Render(renderInfo);

    RenderUi();

    std::array commandBuffersToSubmit = {
        rm.GetRenderer().GetBeginRenderingCommandBuffer(imageIndex),
        m_commandBuffers[imageIndex],
        rm.GetImGuiRenderer().PrepareCommandBuffer(imageIndex),
        rm.GetRenderer().GetEndRenderingCommandBuffer(imageIndex)
    };
    rm.GetQueue().SubmitRenderCommands(commandBuffersToSubmit.size(), commandBuffersToSubmit.data());

    rm.GetQueue().Present(imageIndex);
}

void GameApplication::RenderUi() const
{
    ImGuiIO& io = ImGui::GetIO();

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();

    ImGui::Begin(GetApplicationName(), nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

    ImGui::End();

    ImGui::Render();
}

bool GameApplication::ShouldClose() const
{
    return m_engine->GetWindow().ShouldClose();
}

const char* GameApplication::GetApplicationName() const
{
    return "MyApp";
}

void GameApplication::OnWindowResize() const
{
    RecordCommandBuffers();
}

void GameApplication::BeginRendering(VkCommandBuffer commandBuffer, glm::u32 imageIndex) const
{
    auto& rm = m_engine->GetRenderingManager();

    VkClearValue clearColor = {
        .color = {0.0f, 0.0f, 0.0f, 1.0f},
    };

    VkClearValue clearDepthValue = {
        .depthStencil = {.depth = 1.0f, .stencil = 0}
    };

    rm.GetRenderer().BeginDynamicRendering(commandBuffer, imageIndex, &clearColor, &clearDepthValue);
}

void GameApplication::RecordCommandBuffers() const
{
    auto& rm = m_engine->GetRenderingManager();

    for (int i = 0; i < m_commandBuffers.size(); ++i)
    {
        VkCommandBuffer commandBuffer = m_commandBuffers[i];
        VkCommandBufferUsageFlags flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        rm.GetCommandManager().BeginCommandBuffer(commandBuffer, flags);

        BeginRendering(commandBuffer, i);

        m_graphicsPipeline->CmdBindTo(commandBuffer);
        m_descriptorManager->CmdBind(commandBuffer, i, m_graphicsPipeline->GetLayout(), 0, 0);
        m_descriptorManager->CmdBind(commandBuffer, i, m_graphicsPipeline->GetLayout(), 1, 1);

        m_graphicsPipeline->CmdSetViewportToWindowSize(commandBuffer, m_engine->GetWindow().GetDimensions());

        m_models->CmdBindIndexBuffer(commandBuffer);
        m_models->CmdRenderModels(commandBuffer, *m_graphicsPipeline, 0);

        vkCmdEndRendering(commandBuffer);

        rm.GetCommandManager().EndCommandBuffer(commandBuffer);
    }

    spdlog::info("Command buffers recorded");
}

std::vector<std::string> GameApplication::CreateModels()
{
    std::vector<std::string> imagesToLoad;
    std::vector<Model> models;

    auto fileName = std::format("{}/Cube.obj", ASSETS_DIRECTORY);
    models.push_back(ModelLoader::LoadModel(fileName.c_str(), imagesToLoad));

    m_models = std::make_unique<SceneModels>(
        m_engine->GetRenderingManager(),
        models
    );

    return imagesToLoad;
}

void GameApplication::SetupDescriptors()
{
    DescriptorSetLayoutList layouts(m_engine->GetRenderingManager().GetSwapchain().GetNumImages());
    {
        // Constant set
        ASSERT(layouts.Size() == SPIRE_SHADER_BINDINGS_CONSTANT_SET);

        DescriptorSetLayout layout;

        // ModelVertex buffer
        layout.push_back(m_models->GetDescriptor(SPIRE_SHADER_BINDINGS_VERTEX_SSBO_BINDING));

        // Images
        layout.push_back(m_sceneImages->GetDescriptor(SPIRE_SHADER_BINDINGS_MODEL_IMAGES_BINDING));

        layouts.Push(layout);
    }

    {
        ASSERT(layouts.Size() == SPIRE_SHADER_BINDINGS_PER_FRAME_SET);
        // Per frame set
        PerImageDescriptorSetLayout layout;

        // Camera
        layout.push_back(m_camera->GetDescriptor(SPIRE_SHADER_BINDINGS_CAMERA_UBO_BINDING));

        layouts.Push(layout);
    }

    // Create descriptor manager
    m_descriptorManager = std::make_unique<DescriptorManager>(m_engine->GetRenderingManager(), layouts);
}

void GameApplication::SetupGraphicsPipeline()
{
    auto& rm = m_engine->GetRenderingManager();

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
