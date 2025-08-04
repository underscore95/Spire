#include "Renderer.h"

#include "RenderingManager.h"
#include "Swapchain.h"
#include "TextureManager.h"
#include "Engine/Window/Window.h"

Renderer::Renderer(RenderingManager& renderingManager,
                   const Window& window)
    : m_renderingManager(renderingManager),
      m_window(window)
{
}

void Renderer::BeginRendering(VkCommandBuffer commandBuffer, int imageIndex, const VkClearValue* clearColor,
                              const VkClearValue* depthValue) const
{
    VkRenderingAttachmentInfoKHR colorAttachment = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
        .pNext = nullptr,
        .imageView = m_renderingManager.GetSwapchain().GetImageView(imageIndex),
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .resolveMode = VK_RESOLVE_MODE_NONE,
        .resolveImageView = VK_NULL_HANDLE,
        .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .loadOp = clearColor ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE
    };

    if (clearColor)
    {
        colorAttachment.clearValue = *clearColor;
    }

    VkRenderingAttachmentInfo depthAttachment = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .pNext = nullptr,
        .imageView = m_renderingManager.GetDepthImage(imageIndex).ImageView,
        .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        .resolveMode = VK_RESOLVE_MODE_NONE,
        .resolveImageView = VK_NULL_HANDLE,
        .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .loadOp = depthValue ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    };

    if (depthValue)
    {
        depthAttachment.clearValue = *depthValue;
    }

    VkRenderingInfoKHR renderingInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
        .renderArea = {{0, 0}, {m_window.GetDimensions().x, m_window.GetDimensions().y}},
        .layerCount = 1,
        .viewMask = 0,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachment,
        .pDepthAttachment = &depthAttachment
    };

    vkCmdBeginRendering(commandBuffer, &renderingInfo);
}
