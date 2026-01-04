#pragma once

#include "pch.h"

namespace Spire
{
    class Engine;
    class RenderingManager;

    class VulkanQueue
    {
    public:
        VulkanQueue(RenderingManager& renderingManager,
                    Engine& engine,
                    VkDevice device,
                    VkSwapchainKHR swapchain,
                    glm::u32 queueFamily,
                    glm::u32 queueIndex);

        ~VulkanQueue();

    public:
        [[nodiscard]] glm::u32 AcquireNextImage() const;

        void SubmitImmediate(VkCommandBuffer commandBuffer) const;

        // Submit and trigger a fence
        void Submit(VkCommandBuffer commandBuffer, VkFence fence) const;

        void SubmitRenderCommand(VkCommandBuffer commandBuffer);
        void SubmitRenderCommands(glm::u32 count, VkCommandBuffer* commandBuffers);

        void Present(glm::u32 imageIndex);

        void WaitIdle() const;

        [[nodiscard]] VkQueue GetQueueHandle() const;

    public:
        const glm::u32 INVALID_IMAGE_INDEX = -1; // underflow

    private:
        RenderingManager& m_renderingManager;
        Engine& m_engine;
        VkDevice m_device = VK_NULL_HANDLE;
        VkSwapchainKHR m_swapchain;
        VkQueue m_queue = VK_NULL_HANDLE;
        std::vector<VkSemaphore> m_renderCompleteSemaphore;
        std::vector<VkSemaphore> m_presentCompleteSemaphore;
        std::vector<VkFence> m_framesInFlightFences;
        glm::u32 m_currentFrame = 0;
    };
}