#pragma once

#include <glm/fwd.hpp>
#include <vulkan/vulkan.h>

class RenderingManager;

class VulkanQueue
{
public:
    VulkanQueue(RenderingManager& renderingManager,
                VkDevice device,
                VkSwapchainKHR swapChain,
                glm::u32 queueFamily,
                glm::u32 queueIndex);

    ~VulkanQueue();

public:
    [[nodiscard]] glm::u32 AcquireNextImage() const;

    void SubmitSync(VkCommandBuffer commandBuffer) const;

    void SubmitAsync(VkCommandBuffer commandBuffer);
    void SubmitAsync(glm::u32 count, VkCommandBuffer* commandBuffers);

    void Present(glm::u32 imageIndex);

    void WaitUntilExecutedAll() const;

    [[nodiscard]] VkQueue GetQueueHandle() const;

public:
    const glm::u32 INVALID_IMAGE_INDEX = -1; // underflow

private:
    void CreateSemaphores();

private:
    RenderingManager& m_renderingManager;
    VkDevice m_device = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
    VkQueue m_queue = VK_NULL_HANDLE;
    VkSemaphore m_renderCompleteSemaphore = VK_NULL_HANDLE;
    VkSemaphore m_presentCompleteSemaphore = VK_NULL_HANDLE;
};
