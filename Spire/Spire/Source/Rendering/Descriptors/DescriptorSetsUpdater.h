#pragma once
#include "pch.h"
#include "DescriptorSet.h"

namespace Spire {
    union DescriptorInfo {
        VkDescriptorBufferInfo BufferInfo;
        VkDescriptorImageInfo ImageInfo;
    };

    class DescriptorSetsUpdater {
    public:
        DescriptorSetsUpdater(VkDevice device, glm::u32 numDescriptorSets, const DescriptorSet *descriptorSets);

        ~DescriptorSetsUpdater();

        void Update();

        [[nodiscard]] bool IsBuffer(VkDescriptorType resourceType) const;

        [[nodiscard]] bool IsImageSampler(VkDescriptorType resourceType) const;

        [[nodiscard]] bool IsSupportedResourceType(VkDescriptorType resourceType) const;

    private:
        VkDevice m_device;
        std::vector<VkWriteDescriptorSet> m_writeDescriptorSets;
        std::vector<DescriptorInfo> m_descriptorInfos;
        bool m_hasUpdatedAtLeastOnce = false;
    };
}
