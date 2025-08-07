#include "GameApplication.h"

#include <libassert/assert.hpp>
#include <spdlog/spdlog.h>
#include <glm/gtc/matrix_transform.hpp>

#include "Engine/Rendering/ImGuiRenderer.h"
#include "Engine/Rendering/RenderingDeviceManager.h"

void GameApplication::Start(Engine& engine)
{
    m_engine = &engine;

    auto& rm = m_engine->GetRenderingManager();

    // Camera
    m_camera = std::make_unique<Camera>(m_engine->GetWindow());

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

    // Buffers
    CreateStorageBufferForVertices();
    CreateUniformBuffers();

    // Textures
    std::string textureFile = std::format("{}/test.png", ASSETS_DIRECTORY);
    m_texture = rm.GetTextureManager().CreateImageFromFile(textureFile.c_str());

    // Pipeline
    SetupGraphicsPipeline();

    // Command buffers
    m_commandBuffers.resize(rm.GetSwapchain().GetNumImages());
    rm.GetCommandManager().CreateCommandBuffers(rm.GetSwapchain().GetNumImages(), m_commandBuffers.data());
    RecordCommandBuffers();
}

GameApplication::~GameApplication()
{
    auto& rm = m_engine->GetRenderingManager();

    rm.GetQueue().WaitIdle();
    rm.GetCommandManager().FreeCommandBuffers(m_commandBuffers.size(), m_commandBuffers.data());
    spdlog::info("Freed command buffers");

    rm.GetBufferManager().DestroyBuffer(m_vertexStorageBuffer);
    for (int i = 0; i < m_uniformBuffers.size(); i++)
    {
        rm.GetBufferManager().DestroyBuffer(m_uniformBuffers[i]);
    }

    rm.GetTextureManager().DestroyImage(m_texture);

    m_graphicsPipeline.reset();

    vkDestroyShaderModule(rm.GetDevice(), m_vertexShader, nullptr);
    vkDestroyShaderModule(rm.GetDevice(), m_fragmentShader, nullptr);
    spdlog::info("Destroyed shaders");
}

void GameApplication::Update()
{
    m_camera->Update(m_engine->GetDeltaTime());
}

void GameApplication::Render()
{
    auto& rm = m_engine->GetRenderingManager();

    rm.GetQueue().WaitIdle();
    glm::u32 imageIndex = rm.GetQueue().AcquireNextImage();
    if (imageIndex == rm.GetQueue().INVALID_IMAGE_INDEX) return;

    UpdateUniformBuffers(imageIndex);

    RenderUi();

    rm.GetQueue().WaitIdle();

    std::array commandBuffersToSubmit = {
        rm.GetRenderer().GetBeginRenderingCommandBuffer(imageIndex),
        m_commandBuffers[imageIndex],
        rm.GetImGuiRenderer().PrepareCommandBuffer(imageIndex),
        rm.GetRenderer().GetEndRenderingCommandBuffer(imageIndex)
    };
    rm.GetQueue().SubmitAsync(commandBuffersToSubmit.size(), commandBuffersToSubmit.data());

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

        m_graphicsPipeline->CmdBindTo(commandBuffer, i);

        m_graphicsPipeline->CmdSetViewportToWindowSize(commandBuffer, m_engine->GetWindow().GetDimensions());

        vkCmdDraw(commandBuffer, 9, 1, 0, 0);

        vkCmdEndRendering(commandBuffer);

        rm.GetCommandManager().EndCommandBuffer(commandBuffer);
    }

    spdlog::info("Command buffers recorded");
}

void GameApplication::CreateStorageBufferForVertices()
{
    struct Vertex
    {
        Vertex(const glm::vec3& p, const glm::vec2& t)
        {
            Pos = p;
            Tex = t;
        }

        glm::vec3 Pos;
        glm::vec2 Tex;
    };

    std::vector vertices = {
        // Quad
        Vertex({-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}), // Bottom left
        Vertex({-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}), // Top left
        Vertex({1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}), // Top right
        Vertex({-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}), // Bottom left
        Vertex({1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}), // Top right
        Vertex({1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}), // Bottom right

        // Triangle
        Vertex({-1.0f, -1.0f, -5.0f}, {0.0f, 0.0f}), // Bottom left
        Vertex({-1.0f, 1.0f, -5.0f}, {0.0f, 1.0f}), // Top left
        Vertex({1.0f, 1.0f, -5.0f}, {1.0f, 1.0f}) // Top right
    };

    m_vertexBufferSize = sizeof(vertices[0]) * vertices.size();
    m_vertexStorageBuffer = m_engine->GetRenderingManager().GetBufferManager().CreateStorageBufferForVertices(
        vertices.data(), m_vertexBufferSize);
}

void GameApplication::CreateUniformBuffers()
{
    auto& rm = m_engine->GetRenderingManager();
    m_uniformBuffers = rm.GetBufferManager().CreateUniformBuffers(sizeof(UniformData));
    spdlog::info("Created uniform buffers");
}

void GameApplication::UpdateUniformBuffers(glm::u32 imageIndex) const
{
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -10.0f));

    glm::mat4 WVP = m_camera->GetProjectionMatrix() * m_camera->GetViewMatrix() * model;
    m_engine->GetRenderingManager().GetBufferManager().UpdateBuffer(m_uniformBuffers[imageIndex], &WVP, sizeof(WVP));
}

void GameApplication::SetupGraphicsPipeline()
{
    auto& rm = m_engine->GetRenderingManager();

    std::vector<PipelineResourceInfo> pipelineResources;

    // Vertex buffer
    pipelineResources.push_back({
        .ResourceType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .Binding = 0,
        .Stages = VK_SHADER_STAGE_VERTEX_BIT,
        .SameResourceForAllImages = true,
        .ResourcePtrs = &m_vertexStorageBuffer
    });

    // Uniform buffers
    pipelineResources.push_back({
        .ResourceType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .Binding = 1,
        .Stages = VK_SHADER_STAGE_VERTEX_BIT,
        .SameResourceForAllImages = false,
        .ResourcePtrs = m_uniformBuffers.data()
    });

    // Texture
    pipelineResources.push_back({
        .ResourceType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .Binding = 2,
        .Stages = VK_SHADER_STAGE_FRAGMENT_BIT,
        .SameResourceForAllImages = true,
        .ResourcePtrs = &m_texture
    });

    m_graphicsPipeline = std::make_unique<GraphicsPipeline>(
        rm.GetDevice(),
        m_vertexShader,
        m_fragmentShader,
        std::make_unique<PipelineDescriptorSetsManager>(rm, pipelineResources),
        rm.GetSwapchain().GetSurfaceFormat().format,
        rm.GetPhysicalDevice().DepthFormat,
        rm
    );
}
