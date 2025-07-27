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
    auto &rm = m_engine->GetRenderingManager();

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
    VkClearColorValue clearColor = {1.0f, 0.0f, 0.0f, 1.0f};

    VkImageSubresourceRange imageRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1
    };

    for (glm::u32 i = 0; i < m_commandBuffers.size(); i++) {
        VkCommandBufferUsageFlags flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        rm.GetCommandManager().BeginCommandBuffer(m_commandBuffers[i], flags);

        vkCmdClearColorImage(m_commandBuffers[i], rm.GetImage(i), VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &imageRange);

        rm.GetCommandManager().EndCommandBuffer(m_commandBuffers[i]);

        spdlog::info("Command buffers recorded");
    }
}
