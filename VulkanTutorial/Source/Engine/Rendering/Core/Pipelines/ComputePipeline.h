#pragma once

#include "Pipeline.h"

class ComputePipeline final : public Pipeline
{
public:
    ComputePipeline(
        VkDevice device,
        VkShaderModule shader,
        const DescriptorManager& descriptorManager,
        RenderingManager& renderingManager,
        glm::u32 pushConstantSize
    );
    ~ComputePipeline() override;

public:
    void CmdBindTo(VkCommandBuffer commandBuffer) const override;

    void CmdDispatch(VkCommandBuffer commandBuffer, glm::u32 x, glm::u32 y, glm::u32 z) const;
};
