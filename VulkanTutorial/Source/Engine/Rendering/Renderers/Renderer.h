#pragma once

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>

class Window;
class RenderingManager;

class Renderer
{
public:
    explicit Renderer(RenderingManager& renderingManager, const Window& window);
    ~Renderer();

public:
    void BeginDynamicRendering(VkCommandBuffer commandBuffer, glm::u32 imageIndex, const VkClearValue* clearColor,
                               const VkClearValue* depthValue) const;

    VkCommandBuffer GetBeginRenderingCommandBuffer(glm::u32 imageIndex) const;
    VkCommandBuffer GetEndRenderingCommandBuffer(glm::u32 imageIndex) const;

    void RecreateCommandBuffers();

private:
    void CreateCommandBuffers();
    void FreeCommandBuffers() const;
    void RecordCommandBuffers() const;

private:
    RenderingManager& m_renderingManager;
    const Window& m_window;
    std::vector<VkCommandBuffer> m_beginRenderingCommandBuffers;
    std::vector<VkCommandBuffer> m_endRenderingCommandBuffers;
};
