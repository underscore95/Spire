#pragma once

#include <vulkan/vulkan.hpp>
#include <glm/fwd.hpp>

class PipelineDescriptorSetsManager;
struct GLFWwindow;
struct VulkanBuffer;

class GraphicsPipeline {
public:
    GraphicsPipeline(VkDevice device, glm::uvec2 windowSize, VkRenderPass renderPass,
                     VkShaderModule vertexShader, VkShaderModule fragmentShader, std::unique_ptr<PipelineDescriptorSetsManager> descriptorSetsManager);

    ~GraphicsPipeline();

public:
    void BindTo(VkCommandBuffer commandBuffer, glm::u32 swapchainImageIndex) const;

private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkPipeline m_pipeline = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    std::unique_ptr<PipelineDescriptorSetsManager> m_descriptorSetsManager = nullptr;
};
