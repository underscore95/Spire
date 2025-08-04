#pragma once

#include <vulkan/vulkan.hpp>

class Window;
class RenderingManager;

class Renderer
{
public:
    explicit Renderer(RenderingManager& renderingManager, const Window& window);

public:
    void BeginRendering(VkCommandBuffer commandBuffer, int imageIndex, const VkClearValue* clearColor,
                        const VkClearValue* depthValue) const;

private:
    RenderingManager& m_renderingManager;
    const Window& m_window;
};
