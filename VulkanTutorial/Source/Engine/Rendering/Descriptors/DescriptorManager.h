#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>
#include "DescriptorSetLayoutList.h"

struct DescriptorSet;
class DescriptorSetsUpdater;
class DescriptorPool;
class RenderingManager;

class DescriptorManager
{
public:
    DescriptorManager(RenderingManager& renderingManager, const DescriptorSetLayoutList& layouts);
    ~DescriptorManager();

public:
    void CmdBind(VkCommandBuffer commandBuffer, glm::u32 currentSwapchainImage, VkPipelineLayout pipelineLayout, glm::u32 setIndex, glm
                 ::u32 shaderSetIndex) const;

    const std::vector<VkDescriptorSetLayout>& GetRawLayouts() const;

private:

private:
    RenderingManager& m_renderingManager;
    DescriptorSetLayoutList m_layouts;
    std::unique_ptr<DescriptorPool> m_pool;
    std::unique_ptr<DescriptorSetsUpdater> m_updater;
    std::vector<VkDescriptorSetLayout> m_rawLayouts;
    std::vector<DescriptorSet> m_descriptorSets;
};
