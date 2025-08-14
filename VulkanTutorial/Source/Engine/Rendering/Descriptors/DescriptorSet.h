#pragma once

#include <vector>
#include "Descriptor.h"

struct DescriptorSet
{
    std::vector<Descriptor> Descriptors;
    VkDescriptorSetLayout Layout;
    VkDescriptorSet Handle;

    static std::vector<VkDescriptorSet> ToRawVector(const std::vector<DescriptorSet>& sets)
    {
        std::vector<VkDescriptorSet> raw;
        raw.resize(sets.size());
        for (glm::u32 i = 0; i < sets.size(); i++)
        {
            raw[i] = sets[i].Handle;
        }
        return raw;
    }

    static std::vector<VkDescriptorSetLayout> ToLayoutVector(const std::vector<DescriptorSet>& sets)
    {
        std::vector<VkDescriptorSetLayout> raw;
        raw.resize(sets.size());
        for (glm::u32 i = 0; i < sets.size(); i++)
        {
            raw[i] = sets[i].Layout;
        }
        return raw;
    }

    void CmdBind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, glm::u32 firstSet) const
    {
        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout,
            firstSet,
            1,
            &Handle,
            0,
            nullptr);
    }
};
