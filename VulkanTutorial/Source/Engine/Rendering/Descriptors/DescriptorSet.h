#pragma once
#include "DescriptorSetLayout.h"

class DescriptorSet {
    friend class DescriptorPool;
private:
    explicit DescriptorSet(VkDescriptorSet descriptorSet);

public:
    ~DescriptorSet();

public:
    [[nodiscard]] VkDescriptorSet GetHandle() const;

private:
    VkDescriptorSet m_descriptorSet;
};
