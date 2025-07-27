#pragma once

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>

class RenderingCommandManager {
public:
    RenderingCommandManager(glm::u32 deviceQueueFamily, VkDevice device);

    ~RenderingCommandManager();

public:
    void CreateCommandBuffers(glm::u32 count, VkCommandBuffer *commandBuffers);

    void FreeCommandBuffers(glm::u32 count, const VkCommandBuffer *commandBuffers);

private:
    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    glm::u32 m_allocatedCommandBuffers = 0;
};
