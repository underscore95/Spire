#pragma once

#include <vector>
#include "Descriptor.h"

class DescriptorSetLayout
{
public:
    explicit DescriptorSetLayout(bool isConstantData);

    void Push(const Descriptor& descriptor);

    const std::vector<Descriptor>& GetDescriptors() const;
    bool IsConstant() const;

    glm::u32 Size() const;

private:
    std::vector<Descriptor> m_descriptors;
    bool m_isConstant;
};

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
