#pragma once

#include <vector>

#include "Descriptor.h"

class DescriptorSetLayout
{
public:
    DescriptorSetLayout(VkDevice device, const std::vector<Descriptor>& descriptors);
    ~DescriptorSetLayout();

public:
    [[nodiscard]] VkDescriptorSetLayout GetLayout() const;

public:
    std::vector<Descriptor>::const_iterator begin() const;
    std::vector<Descriptor>::const_iterator end() const;

private:
    void GenerateLayout();

private:
    VkDevice m_device = VK_NULL_HANDLE;
    std::vector<Descriptor> m_descriptors;
    VkDescriptorSetLayout m_layout = VK_NULL_HANDLE;
};
