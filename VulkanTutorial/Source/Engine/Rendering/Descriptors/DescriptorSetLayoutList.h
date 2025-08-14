#pragma once

#include <vector>
#include "Descriptor.h"

typedef std::vector<Descriptor> DescriptorSetLayout;

class DescriptorSetLayoutList
{
public:
    DescriptorSetLayoutList() = default;

    glm::u32 Push(const DescriptorSetLayout& layout);

    const std::vector<DescriptorSetLayout>& GetDescriptorSets() const;

    const DescriptorSetLayout& GetSet(glm::u32 index) const;

    glm::u32 Size() const;

private:
    std::vector<DescriptorSetLayout> m_descriptorSets;
};
