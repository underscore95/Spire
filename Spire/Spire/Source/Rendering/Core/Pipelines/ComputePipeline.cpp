#include "ComputePipeline.h"

#include "Utils/Log.h"
namespace Spire
{
    ComputePipeline::ComputePipeline(
        VkDevice device,
        VkShaderModule shader,
        const DescriptorManager& descriptorManager,
        RenderingManager& renderingManager,
        glm::u32 pushConstantSize) : Pipeline(device, descriptorManager, renderingManager, pushConstantSize)
    {
        VkPipelineShaderStageCreateInfo shaderStageCreateInfo = CreateShaderInfo(shader, VK_SHADER_STAGE_COMPUTE_BIT);

        VkComputePipelineCreateInfo pipelineInfo = {
            .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stage = shaderStageCreateInfo,
            .layout = m_pipelineLayout,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = -1
        };

        VkResult res = vkCreateComputePipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline);
        if (res != VK_SUCCESS)
        {
            error("Failed to create compute pipeline layout");
        }
        else
        {
            info("Compute pipeline created");
        }
    }

    ComputePipeline::~ComputePipeline()
    {
        vkDestroyPipeline(m_device, m_pipeline, nullptr);
        m_pipeline = VK_NULL_HANDLE;
    }

    void ComputePipeline::CmdBindTo(VkCommandBuffer commandBuffer) const
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipeline);
    }

    void ComputePipeline::CmdDispatch(VkCommandBuffer commandBuffer, glm::u32 x, glm::u32 y, glm::u32 z) const
    {
        vkCmdDispatch(commandBuffer, x, y, z);
    }
}