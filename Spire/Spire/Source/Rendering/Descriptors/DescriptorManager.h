#pragma once

#include "pch.h"
#include "DescriptorSetLayoutList.h"

namespace Spire {
    struct DescriptorSet;
    class DescriptorSetsUpdater;
    class DescriptorPool;
    class RenderingManager;

    class DescriptorManager {
    public:
        DescriptorManager(RenderingManager &renderingManager, const DescriptorSetLayoutList &layouts);

        ~DescriptorManager();

    public:
        void CmdBind(VkCommandBuffer commandBuffer, glm::u32 currentSwapchainImage, VkPipelineLayout pipelineLayout, glm::u32 setIndex, glm
                     ::u32 shaderSetIndex) const;

        // update a single descriptor in a descriptor set
        // setIndex - index of the set to update, you will need to call this function multiple times if its a per image set
        // descriptor - new descriptor, this will replace the descriptor with the same binding, you cannot add new descriptors using this function
       void WriteDescriptor(glm::u32 setIndex, const Descriptor &descriptor);

        [[nodiscard]] const std::vector<VkDescriptorSetLayout> &GetRawLayouts() const;

    private:

    private:
        RenderingManager &m_renderingManager;
        DescriptorSetLayoutList m_layouts;
        std::unique_ptr<DescriptorPool> m_pool;
        std::unique_ptr<DescriptorSetsUpdater> m_updater;
        std::vector<VkDescriptorSetLayout> m_rawLayouts;
        std::vector<DescriptorSet> m_descriptorSets;
    };
}
