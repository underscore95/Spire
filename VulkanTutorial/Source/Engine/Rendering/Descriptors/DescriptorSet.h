#pragma once

#include <vector>

#include "Descriptor.h"

class DescriptorSet
{
public:
    DescriptorSet(VkDevice device, const std::vector<Descriptor>& descriptors);
    ~DescriptorSet();

    std::vector<Descriptor>::const_iterator begin() const;
    std::vector<Descriptor>::const_iterator end() const;

private:
    void GenerateLayout();

private:
    VkDevice m_device = VK_NULL_HANDLE;
    std::vector<Descriptor> m_descriptors;
    VkDescriptorSetLayout m_layout = VK_NULL_HANDLE;
};
