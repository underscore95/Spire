#pragma once

#include <vulkan/vulkan.hpp>
#include <glm/fwd.hpp>

struct GLFWwindow;
struct VulkanBuffer;

class GraphicsPipeline {
public:
    GraphicsPipeline(VkDevice device, glm::uvec2 windowSize, VkRenderPass renderPass,
                     VkShaderModule vertexShader, VkShaderModule fragmentShader,
                     const VulkanBuffer &storageBufferForVertices, glm::u32 vertexBufferSize, glm::u32 numSwapchainImages);

    ~GraphicsPipeline();

public:
    void BindTo(VkCommandBuffer commandBuffer, glm::u32 swapchainImageIndex) const;

private:
    void CreateDescriptorPool(glm::u32 numImages);

    void CreateDescriptorSets(glm::u32 numImages, const VulkanBuffer &storageBufferForVertices,
                              glm::u32 vertexBufferSize);

    void CreateDescriptorSetLayout();

    void AllocateDescriptorSets(glm::u32 numImages);

    void UpdateDescriptorSets(glm::u32 numImages, const VulkanBuffer &storageBufferForVertices,
                              glm::u32 vertexBufferSize) const;

private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkPipeline m_pipeline = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> m_descriptorSets;
};
