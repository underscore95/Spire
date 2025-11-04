#include "Renderer.h"

#include "Engine/Rendering/Core/RenderingCommandManager.h"
#include "Engine/Rendering/RenderingManager.h"
#include "Engine/Rendering/Core/RenderingSync.h"
#include "Engine/Rendering/Core/Swapchain.h"
#include "Engine/Rendering/Memory/VulkanImage.h"
#include "Engine/Window/Window.h"

namespace Spire
{
    Renderer::Renderer(RenderingManager& renderingManager,
                       const Window& window)
        : m_renderingManager(renderingManager),
          m_window(window)
    {
        CreateCommandBuffers();
        RecordCommandBuffers();
    }

    Renderer::~Renderer()
    {
        FreeCommandBuffers();
    }

    void Renderer::BeginDynamicRendering(VkCommandBuffer commandBuffer, glm::u32 imageIndex, const VkClearValue* clearColor,
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

    VkCommandBuffer Renderer::GetBeginRenderingCommandBuffer(glm::u32 imageIndex) const
    {
        return m_beginRenderingCommandBuffers[imageIndex];
    }

    VkCommandBuffer Renderer::GetEndRenderingCommandBuffer(glm::u32 imageIndex) const
    {
        return m_endRenderingCommandBuffers[imageIndex];
    }

    void Renderer::RecreateCommandBuffers()
    {
        FreeCommandBuffers();
        CreateCommandBuffers();
        RecordCommandBuffers();
    }

    void Renderer::CreateCommandBuffers()
    {
        m_beginRenderingCommandBuffers.resize(m_renderingManager.GetSwapchain().GetNumImages());
        m_renderingManager.GetCommandManager().CreateCommandBuffers(
            m_beginRenderingCommandBuffers.size(),
            m_beginRenderingCommandBuffers.data());

        m_endRenderingCommandBuffers.resize(m_renderingManager.GetSwapchain().GetNumImages());
        m_renderingManager.GetCommandManager().CreateCommandBuffers(
            m_endRenderingCommandBuffers.size(),
            m_endRenderingCommandBuffers.data());
    }

    void Renderer::FreeCommandBuffers() const
    {
        m_renderingManager.GetCommandManager().FreeCommandBuffers(
            m_beginRenderingCommandBuffers.size(),
            m_beginRenderingCommandBuffers.data());

        m_renderingManager.GetCommandManager().FreeCommandBuffers(
            m_endRenderingCommandBuffers.size(),
            m_endRenderingCommandBuffers.data());
    }

    void Renderer::RecordCommandBuffers() const
    {
        assert(m_beginRenderingCommandBuffers.size() == m_endRenderingCommandBuffers.size());
        for (glm::u32 imageIndex = 0; imageIndex < m_beginRenderingCommandBuffers.size(); ++imageIndex)
        {
            VkCommandBufferUsageFlags flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
            // Begin rendering
            m_renderingManager.GetCommandManager().BeginCommandBuffer(m_beginRenderingCommandBuffers[imageIndex], flags);

            m_renderingManager.GetRenderingSync().ImageMemoryBarrier(
                m_beginRenderingCommandBuffers[imageIndex],
                m_renderingManager.GetSwapchain().GetImage(imageIndex),
                m_renderingManager.GetSwapchain().GetSurfaceFormat().
                                   format,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

            m_renderingManager.GetCommandManager().EndCommandBuffer(m_beginRenderingCommandBuffers[imageIndex]);

            // End rendering
            m_renderingManager.GetCommandManager().BeginCommandBuffer(m_endRenderingCommandBuffers[imageIndex], flags);

            m_renderingManager.GetRenderingSync().ImageMemoryBarrier(
                m_endRenderingCommandBuffers[imageIndex],
                m_renderingManager.GetSwapchain().GetImage(imageIndex),
                m_renderingManager.GetSwapchain().GetSurfaceFormat().
                                   format,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

            m_renderingManager.GetCommandManager().EndCommandBuffer(m_endRenderingCommandBuffers[imageIndex]);
        }
    }
}