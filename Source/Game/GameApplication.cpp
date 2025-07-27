#include "GameApplication.h"

#include <spdlog/spdlog.h>

void GameApplication::Start(Engine &engine) {
    m_engine = &engine;

    auto &rm = m_engine->GetRenderingManager();

    // Command buffers
    m_commandBuffers.resize(rm.GetNumImages());
    rm.GetCommandManager().CreateCommandBuffers(rm.GetNumImages(), m_commandBuffers.data());
    RecordCommandBuffers();
}

GameApplication::~GameApplication() {
    auto &rm = m_engine->GetRenderingManager();

    rm.GetCommandManager().FreeCommandBuffers(m_commandBuffers.size(), m_commandBuffers.data());
}

void GameApplication::Update() {
}

void GameApplication::Render() {
}

bool GameApplication::ShouldClose() const {
    return m_engine->GetWindow().ShouldClose();
}

std::string GameApplication::GetApplicationName() const {
    return "MyApp";
}

void GameApplication::RecordCommandBuffers() const {
    auto &rm = m_engine->GetRenderingManager();
    VkClearColorValue ClearColor = {1.0f, 0.0f, 0.0f, 0.0f};

    VkImageSubresourceRange ImageRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1
    };

    for (glm::u32 i = 0; i < m_commandBuffers.size(); i++) {
        VkCommandBufferUsageFlags flags = 0;
        rm.GetCommandManager().BeginCommandBuffer(m_commandBuffers[i], flags);

        vkCmdClearColorImage(m_commandBuffers[i], rm.GetImage(i), VK_IMAGE_LAYOUT_GENERAL, &ClearColor, 1, &ImageRange);

        VkResult res = vkEndCommandBuffer(m_commandBuffers[i]);
        if (res != VK_SUCCESS) {
            spdlog::error("Failed to end command buffer");
        }

        spdlog::info("Command buffers recorded");
    }
}
