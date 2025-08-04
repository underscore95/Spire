#include "VulkanQueue.h"

#include <spdlog/spdlog.h>
#include <glm/glm.hpp>
#include <libassert/assert.hpp>

#include "RenderingManager.h"
#include "RenderingSync.h"

VulkanQueue::VulkanQueue(
    RenderingManager& renderingManager,
    VkDevice device,
    VkSwapchainKHR swapChain,
    glm::u32 queueFamily,
    glm::u32 queueIndex)
    : m_renderingManager(renderingManager),
      m_device(device),
      m_swapChain(swapChain)
{
    vkGetDeviceQueue(device, queueFamily, queueIndex, &m_queue);

    CreateSemaphores();

    spdlog::info("VulkanQueue initialized");
}

VulkanQueue::~VulkanQueue()
{
    m_renderingManager.GetRenderingSync().DestroySemaphore(m_presentCompleteSemaphore);
    m_renderingManager.GetRenderingSync().DestroySemaphore(m_renderCompleteSemaphore);

    spdlog::info("VulkanQueue shutdown");
}

glm::u32 VulkanQueue::AcquireNextImage() const
{
    glm::u32 imageIndex = 0;
    VkResult res = vkAcquireNextImageKHR(m_device, m_swapChain, UINT64_MAX, m_presentCompleteSemaphore, nullptr,
                                         &imageIndex);
    if (res != VK_SUCCESS)
    {
        spdlog::error("Failed to acquire next free image");
        return INVALID_IMAGE_INDEX;
    }

    return imageIndex;
}

void VulkanQueue::SubmitSync(VkCommandBuffer commandBuffer) const
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

void VulkanQueue::SubmitAsync(VkCommandBuffer commandBuffer)
{
    SubmitAsync(1, &commandBuffer);
}

void VulkanQueue::SubmitAsync(glm::u32 count, VkCommandBuffer* commandBuffers)
{
    VkPipelineStageFlags waitFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &m_presentCompleteSemaphore,
        .pWaitDstStageMask = &waitFlags,
        .commandBufferCount = count,
        .pCommandBuffers = commandBuffers,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &m_renderCompleteSemaphore
    };

    VkResult res = vkQueueSubmit(m_queue, 1, &submitInfo, nullptr);
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
        .pWaitSemaphores = &m_renderCompleteSemaphore,
        .swapchainCount = 1,
        .pSwapchains = &m_swapChain,
        .pImageIndices = &imageIndex
    };

    VkResult res = vkQueuePresentKHR(m_queue, &presentInfo);
    if (res != VK_SUCCESS)
    {
        spdlog::error("Failed to present swapchain image");
    }
}

void VulkanQueue::WaitUntilExecutedAll() const
{
    vkQueueWaitIdle(m_queue);
}

VkQueue VulkanQueue::GetQueueHandle() const
{
    return m_queue;
}

void VulkanQueue::CreateSemaphores()
{
    m_presentCompleteSemaphore = m_renderingManager.GetRenderingSync().CreateSemaphore();
    m_renderCompleteSemaphore = m_renderingManager.GetRenderingSync().CreateSemaphore();
}
