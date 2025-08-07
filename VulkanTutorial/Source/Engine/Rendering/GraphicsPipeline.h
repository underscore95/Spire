#pragma once

#include <vulkan/vulkan.hpp>
#include <glm/fwd.hpp>

class RenderingManager;
class PipelineDescriptorSetsManager;
struct GLFWwindow;
struct VulkanBuffer;

class GraphicsPipeline
{
public:
    GraphicsPipeline(VkDevice device, VkShaderModule vertexShader, VkShaderModule fragmentShader,
                     std::unique_ptr<PipelineDescriptorSetsManager> descriptorSetsManager, VkFormat colorFormat,
                     VkFormat depthFormat, RenderingManager& renderingManager);

    ~GraphicsPipeline();

    void SetViewport(VkCommandBuffer commandBuffer, const VkViewport* viewport, const VkRect2D* scissor) const;
    void SetViewportToWindowSize(VkCommandBuffer commandBuffer,glm::uvec2 windowDimensions) const;

public:
    void BindTo(VkCommandBuffer commandBuffer, glm::u32 swapchainImageIndex) const;

private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkPipeline m_pipeline = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    std::unique_ptr<PipelineDescriptorSetsManager> m_descriptorSetsManager = nullptr;
    RenderingManager& m_renderingManager;
};
