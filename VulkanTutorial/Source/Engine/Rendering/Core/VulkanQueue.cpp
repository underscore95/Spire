#include "VulkanQueue.h"
#include <spdlog/spdlog.h>
#include <glm/glm.hpp>
#include "Engine/Rendering/RenderingManager.h"
#include "RenderingSync.h"
#include "Swapchain.h"
#include "Engine/Core/Engine.h"

namespace Spire
{
    VulkanQueue::VulkanQueue(
        RenderingManager& renderingManager,
        Engine& engine,
        VkDevice device,
        VkSwapchainKHR swapchain,
        glm::u32 queueFamily,
        glm::u32 queueIndex)
        : m_renderingManager(renderingManager),
          m_engine(engine),
          m_device(device),
          m_swapchain(swapchain)
    {
        vkGetDeviceQueue(device, queueFamily, queueIndex, &m_queue);

        // Create fences and semaphores
        m_framesInFlightFences.resize(renderingManager.GetSwapchain().GetNumImages());
        for (glm::u32 i = 0; i < renderingManager.GetSwapchain().GetNumImages(); i++)
        {
            VkFenceCreateInfo fenceInfo = {
                .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                .pNext = nullptr,
                .flags = VK_FENCE_CREATE_SIGNALED_BIT
            };
            vkCreateFence(m_device, &fenceInfo, nullptr, &m_framesInFlightFences[i]);

            m_presentCompleteSemaphore.push_back(m_renderingManager.GetRenderingSync().CreateSemaphore());
            m_renderCompleteSemaphore.push_back(m_renderingManager.GetRenderingSync().CreateSemaphore());
        }

        spdlog::info("VulkanQueue initialized");
    }

    VulkanQueue::~VulkanQueue()
    {
        for (VkSemaphore semaphore : m_presentCompleteSemaphore)
        {
            m_renderingManager.GetRenderingSync().DestroySemaphore(semaphore);
        }
        for (VkSemaphore semaphore : m_renderCompleteSemaphore)
        {
            m_renderingManager.GetRenderingSync().DestroySemaphore(semaphore);
        }
        for (VkFence fence : m_framesInFlightFences)
        {
            vkDestroyFence(m_device, fence, nullptr);
        }

        spdlog::info("VulkanQueue shutdown");
    }

    glm::u32 VulkanQueue::AcquireNextImage() const
    {
        vkWaitForFences(m_device, 1, &m_framesInFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

        glm::u32 imageIndex = 0;
        VkResult res = vkAcquireNextImageKHR(
            m_device,
            m_swapchain,
            UINT64_MAX,
            m_presentCompleteSemaphore[m_currentFrame],
            nullptr,
            &imageIndex);

        vkResetFences(m_device, 1, &m_framesInFlightFences[m_currentFrame]);

        if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR)
        {
            m_engine.OnWindowResize();
            return INVALID_IMAGE_INDEX;
        }
        if (res != VK_SUCCESS)
        {
            spdlog::error("Failed to acquire next free image");
            return INVALID_IMAGE_INDEX;
        }

        return imageIndex;
    }

    void VulkanQueue::SubmitImmediate(VkCommandBuffer commandBuffer) const
    {
        VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = 0,
            .pWaitSemaphores = VK_NULL_HANDLE,
            .pWaitDstStageMask = VK_NULL_HANDLE,
            .commandBufferCount = 1,
            .pCommandBuffers = &commandBuffer,
            .signalSemaphoreCount = 0,
            .pSignalSemaphores = VK_NULL_HANDLE
        };

        VkResult res = vkQueueSubmit(m_queue, 1, &submitInfo, nullptr);
        if (res != VK_SUCCESS)
        {
            spdlog::error("Failed to submit commands to queue sync");
        }
    }

    void VulkanQueue::SubmitRenderCommand(VkCommandBuffer commandBuffer)
    {
        SubmitRenderCommands(1, &commandBuffer);
    }

    void VulkanQueue::SubmitRenderCommands(glm::u32 count, VkCommandBuffer* commandBuffers)
    {
        VkPipelineStageFlags waitFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &m_presentCompleteSemaphore[m_currentFrame],
            .pWaitDstStageMask = &waitFlags,
            .commandBufferCount = count,
            .pCommandBuffers = commandBuffers,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &m_renderCompleteSemaphore[m_currentFrame]
        };

        VkResult res = vkQueueSubmit(m_queue, 1, &submitInfo, m_framesInFlightFences[m_currentFrame]);
        // todo fence might be better on present
        if (res != VK_SUCCESS)
        {
            spdlog::error("Failed to submit commands to queue async");
        }
    }

    void VulkanQueue::Present(glm::u32 imageIndex)
    {
        VkPresentInfoKHR presentInfo = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &m_renderCompleteSemaphore[m_currentFrame],
            .swapchainCount = 1,
            .pSwapchains = &m_swapchain,
            .pImageIndices = &imageIndex
        };

        VkResult res = vkQueuePresentKHR(m_queue, &presentInfo);
        m_currentFrame = (m_currentFrame + 1) % m_renderingManager.GetSwapchain().GetNumImages();
        if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR)
        {
            m_engine.OnWindowResize();
            return;
        }
        if (res != VK_SUCCESS)
        {
            spdlog::error("Failed to present swapchain image");
        }
    }

    void VulkanQueue::WaitIdle() const
    {
        VkResult res = vkQueueWaitIdle(m_queue);
        if (res != VK_SUCCESS)
        {
            spdlog::info("Failed to wait until queue is idle");
        }
    }

    VkQueue VulkanQueue::GetQueueHandle() const
    {
        return m_queue;
    }
}