#include "GameApplication.h"

#include <libassert/assert.hpp>
#include <spdlog/spdlog.h>
#include <glm/gtc/matrix_transform.hpp>

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
    m_texture = rm.GetTextureManager().CreateTexture(textureFile.c_str());

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

    rm.GetQueue().WaitUntilExecutedAll();
    rm.GetCommandManager().FreeCommandBuffers(m_commandBuffers.size(), m_commandBuffers.data());
    spdlog::info("Freed command buffers");

    rm.GetBufferManager().DestroyBuffer(m_vertexStorageBuffer);
    for (int i = 0; i < m_uniformBuffers.size(); i++)
    {
        rm.GetBufferManager().DestroyBuffer(m_uniformBuffers[i]);
    }

    rm.GetTextureManager().DestroyTexture(m_texture);

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

    rm.GetQueue().WaitUntilExecutedAll();
    glm::u32 imageIndex = rm.GetQueue().AcquireNextImage();

    UpdateUniformBuffers(imageIndex);
    rm.GetQueue().WaitUntilExecutedAll();

    rm.GetQueue().SubmitAsync(m_commandBuffers[imageIndex]);

    rm.GetQueue().Present(imageIndex);
}

bool GameApplication::ShouldClose() const
{
    return m_engine->GetWindow().ShouldClose();
}

std::string GameApplication::GetApplicationName() const
{
    return "MyApp";
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

    VkRenderingAttachmentInfoKHR colorAttachment = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
        .pNext = nullptr,
        .imageView = rm.GetSwapchain().GetImageView(imageIndex),
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .resolveMode = VK_RESOLVE_MODE_NONE,
        .resolveImageView = VK_NULL_HANDLE,
        .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = clearColor
    };

    VkRenderingAttachmentInfo depthAttachment = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .pNext = nullptr,
        .imageView = rm.GetDepthImage(imageIndex).ImageView,
        .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        .resolveMode = VK_RESOLVE_MODE_NONE,
        .resolveImageView = VK_NULL_HANDLE,
        .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = clearDepthValue,
    };

    glm::uvec2 windowDimensions = m_engine->GetWindow().GetDimensions();
    VkRenderingInfoKHR renderingInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
        .renderArea = {{0, 0}, {windowDimensions.x, windowDimensions.y}},
        .layerCount = 1,
        .viewMask = 0,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachment,
        .pDepthAttachment = &depthAttachment
    };

    vkCmdBeginRendering(commandBuffer, &renderingInfo);
}

void GameApplication::RecordCommandBuffers() const
{
    auto& rm = m_engine->GetRenderingManager();

    for (int i = 0; i < m_commandBuffers.size(); ++i)
    {
        VkCommandBuffer commandBuffer = m_commandBuffers[i];
        VkCommandBufferUsageFlags flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        rm.GetCommandManager().BeginCommandBuffer(commandBuffer, flags);

        rm.ImageMemoryBarrier(commandBuffer, rm.GetSwapchain().GetImage(i),
                              rm.GetSwapchain().GetSwapChainSurfaceFormat().format,
                              VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

        BeginRendering(commandBuffer, i);

        m_graphicsPipeline->BindTo(commandBuffer, i);

        vkCmdDraw(commandBuffer, 9, 1, 0, 0);

        vkCmdEndRendering(commandBuffer);

        rm.ImageMemoryBarrier(commandBuffer, rm.GetSwapchain().GetImage(i),
                              rm.GetSwapchain().GetSwapChainSurfaceFormat().format,
                              VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

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
        m_engine->GetWindow().GetDimensions(),
        m_vertexShader,
        m_fragmentShader,
        std::make_unique<PipelineDescriptorSetsManager>(rm, pipelineResources),
        rm.GetSwapchain().GetSwapChainSurfaceFormat().format,
        rm.GetPhysicalDevice().DepthFormat
    );
}
