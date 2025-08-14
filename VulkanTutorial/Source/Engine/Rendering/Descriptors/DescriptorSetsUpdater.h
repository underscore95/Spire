#pragma once
#include <vector>
#include <glm/glm.hpp>

#include "DescriptorSetLayout.h"

union DescriptorInfo
{
    VkDescriptorBufferInfo BufferInfo;
    VkDescriptorImageInfo ImageInfo;
};

class DescriptorSetsUpdater
{
public:
    DescriptorSetsUpdater(VkDevice device, glm::u32 numDescriptorSets, const DescriptorSetLayout* descriptorSets);
    ~DescriptorSetsUpdater();

    void Update();

private:
    bool IsBuffer(VkDescriptorType resourceType) const;

    bool IsTextureSampler(VkDescriptorType resourceType) const;

    bool IsSupportedResourceType(VkDescriptorType resourceType) const;

private:
    VkDevice m_device;
    std::vector<VkWriteDescriptorSet> m_writeDescriptorSets;
    std::vector<DescriptorInfo> m_descriptorInfos;
    bool m_hasUpdatedAtLeastOnce = false;
};
