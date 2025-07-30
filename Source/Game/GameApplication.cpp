#include "GameApplication.h"

#include <libassert/assert.hpp>
#include <spdlog/spdlog.h>
#include <glm/gtc/matrix_transform.hpp>

void GameApplication::Start(Engine &engine) {
    m_engine = &engine;

    auto &rm = m_engine->GetRenderingManager();

    // Render pass
    m_renderPass = rm.CreateSimpleRenderPass();
    rm.CreateFramebuffers(m_frameBuffers, m_renderPass, m_engine->GetWindow().GetSize());

    // Shaders
    ShaderCompiler compiler(rm.GetDevice());
    spdlog::info("Created shader compiler");
    m_vertexShader = compiler.CreateShaderModuleFromText("test.vert");
    m_fragmentShader = compiler.CreateShaderModuleFromText("test.frag");
    DEBUG_ASSERT(m_vertexShader != VK_NULL_HANDLE);
    DEBUG_ASSERT(m_fragmentShader != VK_NULL_HANDLE);
    spdlog::info("Created shaders");

    // Buffers
    CreateStorageBufferForVertices();
    CreateUniformBuffers();

    // Pipeline
    m_graphicsPipeline = std::make_unique<GraphicsPipeline>(rm.GetDevice(), m_engine->GetWindow().GetSize(),
                                                            m_renderPass, m_vertexShader, m_fragmentShader,
                                                            m_vertexStorageBuffer, m_vertexBufferSize,
                                                            rm.GetNumImages(), m_uniformBuffers, sizeof(UniformData));

    // Command buffers
    m_commandBuffers.resize(rm.GetNumImages());
    rm.GetCommandManager().CreateCommandBuffers(rm.GetNumImages(), m_commandBuffers.data());
    RecordCommandBuffers();
}

GameApplication::~GameApplication() {
    auto &rm = m_engine->GetRenderingManager();

    rm.GetQueue().WaitUntilExecutedAll();
    rm.GetCommandManager().FreeCommandBuffers(m_commandBuffers.size(), m_commandBuffers.data());
    spdlog::info("Freed command buffers");

    rm.GetBufferManager().DestroyBuffer(m_vertexStorageBuffer);

    m_graphicsPipeline.reset();

    for (int i = 0; i < m_frameBuffers.size(); i++) {
        vkDestroyFramebuffer(rm.GetDevice(), m_frameBuffers[i], nullptr);
    }
    spdlog::info("Destroyed framebuffers");
    vkDestroyShaderModule(rm.GetDevice(), m_vertexShader, nullptr);
    vkDestroyShaderModule(rm.GetDevice(), m_fragmentShader, nullptr);
    spdlog::info("Destroyed shaders");
    vkDestroyRenderPass(rm.GetDevice(), m_renderPass, nullptr);
    spdlog::info("Destroyed render pass");

    for (int i = 0; i < m_uniformBuffers.size(); i++) {
        rm.GetBufferManager().DestroyBuffer(m_uniformBuffers[i]);
    }
}

void GameApplication::Update() {
}

void GameApplication::Render() {
    auto &rm = m_engine->GetRenderingManager();

    rm.GetQueue().WaitUntilExecutedAll();
    glm::u32 imageIndex = rm.GetQueue().AcquireNextImage();

    UpdateUniformBuffers(imageIndex);
    rm.GetQueue().WaitUntilExecutedAll();

    rm.GetQueue().SubmitAsync(m_commandBuffers[imageIndex]);

    rm.GetQueue().Present(imageIndex);
}

bool GameApplication::ShouldClose() const {
    return m_engine->GetWindow().ShouldClose();
}

std::string GameApplication::GetApplicationName() const {
    return "MyApp";
}

void GameApplication::RecordCommandBuffers() const {
    auto &rm = m_engine->GetRenderingManager();
    VkClearColorValue clearColor = {1.0f, 0.0f, 0.0f, 0.0f};
    VkClearValue clearValue;
    clearValue.color = clearColor;

    for (int i = 0; i < m_commandBuffers.size(); ++i) {
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
                    .width = m_engine->GetWindow().GetSize().x,
                    .height = m_engine->GetWindow().GetSize().y
                }
            },
            .clearValueCount = 1,
            .pClearValues = &clearValue,
        };

        VkCommandBufferUsageFlags flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        rm.GetCommandManager().BeginCommandBuffer(m_commandBuffers[i], flags);

        vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        m_graphicsPipeline->BindTo(m_commandBuffers[i], i);
        vkCmdDraw(m_commandBuffers[i], 3, 1, 0, 0);

        vkCmdEndRenderPass(m_commandBuffers[i]);

        rm.GetCommandManager().EndCommandBuffer(m_commandBuffers[i]);
    }

    spdlog::info("Command buffers recorded");
}

void GameApplication::CreateStorageBufferForVertices() {
    struct Vertex {
        Vertex(const glm::vec3 &p, const glm::vec2 &t) {
            Pos = p;
            Tex = t;
        }

        glm::vec3 Pos;
        glm::vec2 Tex;
    };

    std::vector Vertices = {
        Vertex({-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}), // top left
        Vertex({1.0f, -1.0f, 0.0f}, {0.0f, 1.0f}), // top right
        Vertex({0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}) // bottom middle
    };

    m_vertexBufferSize = sizeof(Vertices[0]) * Vertices.size();
    m_vertexStorageBuffer = m_engine->GetRenderingManager().GetBufferManager().CreateStorageBufferForVertices(
        Vertices.data(), m_vertexBufferSize);
}

void GameApplication::CreateUniformBuffers() {
    auto &rm = m_engine->GetRenderingManager();
    m_uniformBuffers = rm.GetBufferManager().CreateUniformBuffers(sizeof(UniformData));
    spdlog::info("Created uniform buffers");
}

void GameApplication::UpdateUniformBuffers(glm::u32 imageIndex) const {
    static float foo = 0.0f;
    auto Rotate = glm::mat4(1.0);
    Rotate = glm::rotate(Rotate, glm::radians(foo), glm::normalize(glm::vec3(0.0f, 0.0f, 1.0f)));
    foo += 0.001f;

    glm::mat4 WVP = Rotate;
    m_engine->GetRenderingManager().GetBufferManager().UpdateBuffer(m_uniformBuffers[imageIndex], &WVP, sizeof(WVP));
}
