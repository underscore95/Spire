#include "GameApplication.h"

#include <libassert/assert.hpp>
#include <spdlog/spdlog.h>
#include <glm/gtc/matrix_transform.hpp>

void GameApplication::Start(Engine& engine)
{
    m_engine = &engine;

    auto& rm = m_engine->GetRenderingManager();

    // Camera
    m_camera = std::make_unique<Camera>(m_engine->GetWindow());

    // Render pass
    m_renderPass = rm.CreateSimpleRenderPass();
    rm.CreateFramebuffers(m_frameBuffers, m_renderPass, m_engine->GetWindow().GetDimensions());

    // Shaders
    ShaderCompiler compiler(rm.GetDevice());
    spdlog::info("Created shader compiler");
    m_vertexShader = compiler.CreateShaderModuleFromText(std::format("{}/test.vert", ASSETS_DIRECTORY));
    m_fragmentShader = compiler.CreateShaderModuleFromText(std::format("{}/test.frag", ASSETS_DIRECTORY));
    DEBUG_ASSERT(m_vertexShader != VK_NULL_HANDLE);
    DEBUG_ASSERT(m_fragmentShader != VK_NULL_HANDLE);
    spdlog::info("Created shaders");

    // Buffers
    CreateStorageBufferForVertices();
    CreateUniformBuffers();

    // Textures
    std::string textureFile = std::format("{}/test.png", ASSETS_DIRECTORY);
    m_texture = rm.GetTextureManager().CreateTexture(textureFile.c_str());

    // Pipeline
    SetupGraphicsPipeline();

    // Command buffers
    m_commandBuffers.resize(rm.GetNumImages());
    rm.GetCommandManager().CreateCommandBuffers(rm.GetNumImages(), m_commandBuffers.data());
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

    for (int i = 0; i < m_frameBuffers.size(); i++)
    {
        vkDestroyFramebuffer(rm.GetDevice(), m_frameBuffers[i], nullptr);
    }
    spdlog::info("Destroyed framebuffers");
    vkDestroyShaderModule(rm.GetDevice(), m_vertexShader, nullptr);
    vkDestroyShaderModule(rm.GetDevice(), m_fragmentShader, nullptr);
    spdlog::info("Destroyed shaders");
    vkDestroyRenderPass(rm.GetDevice(), m_renderPass, nullptr);
    spdlog::info("Destroyed render pass");
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

void GameApplication::RecordCommandBuffers() const
{
    auto& rm = m_engine->GetRenderingManager();
    VkClearColorValue clearColor = {1.0f, 0.0f, 0.0f, 0.0f};
    std::array clearValues = {
        VkClearValue{
            .color = clearColor
        },
        VkClearValue{
            .depthStencil = {1.0f, 0}
        }
    };

    for (int i = 0; i < m_commandBuffers.size(); ++i)
    {
        VkRenderPassBeginInfo renderPassBeginInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext = nullptr,
            .renderPass = m_renderPass,
            .framebuffer = m_frameBuffers[i],
            .renderArea = {
                .offset = {
                    .x = 0,
                    .y = 0
                },
                .extent = {
                    .width = m_engine->GetWindow().GetDimensions().x,
                    .height = m_engine->GetWindow().GetDimensions().y
                }
            },
            .clearValueCount = clearValues.size(),
            .pClearValues = clearValues.data(),
        };

        VkCommandBufferUsageFlags flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        rm.GetCommandManager().BeginCommandBuffer(m_commandBuffers[i], flags);

        vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        m_graphicsPipeline->BindTo(m_commandBuffers[i], i);
        vkCmdDraw(m_commandBuffers[i], 9, 1, 0, 0);

        vkCmdEndRenderPass(m_commandBuffers[i]);

        rm.GetCommandManager().EndCommandBuffer(m_commandBuffers[i]);
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
        rm.GetDevice(), m_engine->GetWindow().GetDimensions(),
        m_renderPass, m_vertexShader, m_fragmentShader,
        std::make_unique<PipelineDescriptorSetsManager>(
            rm, pipelineResources));
}
