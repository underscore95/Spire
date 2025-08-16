#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>
#include <glm/glm.hpp>

class DescriptorManager;
class RenderingManager;

class Pipeline
{
protected:
    Pipeline(
        VkDevice device,
        const DescriptorManager& descriptorManager,
        RenderingManager& renderingManager,
        glm::u32 pushConstantSize
    );

public:
    virtual ~Pipeline();

public:
    void CmdSetPushConstants(VkCommandBuffer commandBuffer, const void* data, glm::u32 size, glm::u32 offset = 0) const;

    [[nodiscard]] VkPipelineLayout GetLayout() const;

protected:
    std::array<VkPipelineShaderStageCreateInfo, 2> CreateVertFragShaderInfo(
        VkShaderModule vertexShader,
        VkShaderModule fragmentShader
    ) const;

    VkPipelineShaderStageCreateInfo CreateShaderInfo(VkShaderModule shader, VkShaderStageFlagBits stage, const char* entryPoint = "main") const;

private:
    void CreatePipelineLayout();

protected:
    VkDevice m_device = VK_NULL_HANDLE;
    VkPipeline m_pipeline = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    RenderingManager& m_renderingManager;
    const glm::u32 m_pushConstantSize;
    std::vector<VkDescriptorSetLayout> m_descriptorSetLayouts;
};
