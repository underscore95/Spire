#pragma once

#include <vulkan/vulkan.hpp>
#include <glm/fwd.hpp>

class RenderingManager;
struct GLFWwindow;
struct VulkanBuffer;
class DescriptorSetLayout;

class GraphicsPipeline
{
public:
    GraphicsPipeline(VkDevice device, VkShaderModule vertexShader, VkShaderModule fragmentShader,
                     const std::vector<DescriptorSetLayout>& descriptorSets, VkFormat colorFormat,
                     VkFormat depthFormat, RenderingManager& renderingManager, glm::u32 pushConstantSize);

    ~GraphicsPipeline();

public:
    void CmdSetViewport(VkCommandBuffer commandBuffer, const VkViewport* viewport, const VkRect2D* scissor) const;
    void CmdSetViewportToWindowSize(VkCommandBuffer commandBuffer, glm::uvec2 windowDimensions) const;

    void CmdBindTo(VkCommandBuffer commandBuffer) const;

    void CmdSetPushConstants(VkCommandBuffer commandBuffer, const void* data, glm::u32 size, glm::u32 offset = 0) const;

    [[nodiscard]] VkPipelineLayout GetLayout() const;

private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkPipeline m_pipeline = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    RenderingManager& m_renderingManager;
    const glm::u32 m_pushConstantSize;
    std::vector<VkDescriptorSetLayout> m_descriptorSetLayouts;
};
