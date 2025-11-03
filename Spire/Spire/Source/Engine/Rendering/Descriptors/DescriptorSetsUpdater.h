#pragma once
#include <vector>
#include <glm/glm.hpp>

#include "DescriptorSet.h"

namespace Spire
{
    union DescriptorInfo
    {
        VkDescriptorBufferInfo BufferInfo;
        VkDescriptorImageInfo ImageInfo;
    };

    class DescriptorSetsUpdater
    {
    public:
        DescriptorSetsUpdater(VkDevice device, glm::u32 numDescriptorSets, const DescriptorSet* descriptorSets);
        ~DescriptorSetsUpdater();

        void Update();

    private:
        bool IsBuffer(VkDescriptorType resourceType) const;

        bool IsImageSampler(VkDescriptorType resourceType) const;

        bool IsSupportedResourceType(VkDescriptorType resourceType) const;

    private:
        VkDevice m_device;
        std::vector<VkWriteDescriptorSet> m_writeDescriptorSets;
        std::vector<DescriptorInfo> m_descriptorInfos;
        bool m_hasUpdatedAtLeastOnce = false;
    };
}