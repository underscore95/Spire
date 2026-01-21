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

        [[nodiscard]] static bool IsBuffer(VkDescriptorType resourceType);

        [[nodiscard]] static bool IsImageSampler(VkDescriptorType resourceType);

        [[nodiscard]] static bool IsSupportedResourceType(VkDescriptorType resourceType);

    private:
        VkDevice m_device;
        std::vector<VkWriteDescriptorSet> m_writeDescriptorSets;
        std::vector<DescriptorInfo> m_descriptorInfos;
        bool m_hasUpdatedAtLeastOnce = false;
    };
}
