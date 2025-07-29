#include "GameApplication.h"

#include <libassert/assert.hpp>
#include <spdlog/spdlog.h>

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

    // Pipeline
    m_graphicsPipeline = std::make_unique<GraphicsPipeline>(rm.GetDevice(), m_engine->GetWindow().GetSize(),
                                                            m_renderPass, m_vertexShader, m_fragmentShader);

    // Command buffers
    m_commandBuffers.resize(rm.GetNumImages());
    rm.GetCommandManager().CreateCommandBuffers(rm.GetNumImages(), m_commandBuffers.data());
    RecordCommandBuffers();
}

GameApplication::~GameApplication() {
    auto &rm = m_engine->GetRenderingManager();

    rm.GetQueue().WaitIdle();
    rm.GetCommandManager().FreeCommandBuffers(m_commandBuffers.size(), m_commandBuffers.data());
    spdlog::info("Freed command buffers");

m_graphicsPipeline.release();

    for (int i = 0; i < m_frameBuffers.size(); i++) {
        vkDestroyFramebuffer(rm.GetDevice(), m_frameBuffers[i], nullptr);
    }
    spdlog::info("Destroyed framebuffers");
    vkDestroyShaderModule(rm.GetDevice(), m_vertexShader, nullptr);
    vkDestroyShaderModule(rm.GetDevice(), m_fragmentShader, nullptr);
    spdlog::info("Destroyed shaders");
    vkDestroyRenderPass(rm.GetDevice(), m_renderPass, nullptr);
    spdlog::info("Destroyed render pass");
}

void GameApplication::Update() {
}

void GameApplication::Render() {
    auto &rm = m_engine->GetRenderingManager();

    rm.GetQueue().WaitIdle();
    glm::u32 imageIndex = rm.GetQueue().AcquireNextImage();

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

        m_graphicsPipeline->BindTo(m_commandBuffers[i]);
        vkCmdDraw(m_commandBuffers[i], 3,1,0,0);

        vkCmdEndRenderPass(m_commandBuffers[i]);

        rm.GetCommandManager().EndCommandBuffer(m_commandBuffers[i]);
    }

    spdlog::info("Command buffers recorded");
}
