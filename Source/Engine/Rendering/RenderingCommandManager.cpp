#include "RenderingCommandManager.h"

#include <libassert/assert.hpp>
#include <spdlog/spdlog.h>

RenderingCommandManager::RenderingCommandManager(
    glm::u32 deviceQueueFamily,
    VkDevice device) : m_device(device) {
    // Create command pool
    VkCommandPoolCreateInfo cmdPoolCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueFamilyIndex = deviceQueueFamily
    };

    VkResult res = vkCreateCommandPool(m_device, &cmdPoolCreateInfo, nullptr, &m_commandPool);
    if (res != VK_SUCCESS) {
        spdlog::error("Failed to create command pool");
    } else {
        spdlog::info("Command buffer pool created");
    }
}

RenderingCommandManager::~RenderingCommandManager() {
    DEBUG_ASSERT(m_allocatedCommandBuffers==0) ; // Didn't free all command buffers
    if (m_commandPool) {
        vkDestroyCommandPool(m_device, m_commandPool, nullptr);
        spdlog::info("Destroyed command pool");
    }
}

void RenderingCommandManager::CreateCommandBuffers(glm::u32 count, VkCommandBuffer *commandBuffers) {
    VkCommandBufferAllocateInfo cmdBufAllocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = m_commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = count
    };

    VkResult res = vkAllocateCommandBuffers(m_device, &cmdBufAllocInfo, commandBuffers);
    if (res != VK_SUCCESS) {
        spdlog::error("Failed to create {} command buffers", count);
    } else {
        spdlog::info("Created {} command buffers", count);
        m_allocatedCommandBuffers += count;
    }
}

void RenderingCommandManager::FreeCommandBuffers(glm::u32 count, const VkCommandBuffer *commandBuffers) {
    vkFreeCommandBuffers(m_device, m_commandPool, count, commandBuffers);
    DEBUG_ASSERT(m_allocatedCommandBuffers>=count); // Freed too many times (or a create call failed)
    m_allocatedCommandBuffers -= count;
}
