#include "DescriptorSetLayout.h"
#include <libassert/assert.hpp>

DescriptorSetLayout::DescriptorSetLayout(bool isConstant)
    : m_isConstant(isConstant)
{
}

void DescriptorSetLayout::Push(const Descriptor& descriptor)
{
    m_descriptors.push_back(descriptor);
}

const std::vector<Descriptor>& DescriptorSetLayout::GetDescriptors() const
{
    return m_descriptors;
}

bool DescriptorSetLayout::IsConstant() const
{
    return m_isConstant;
}

glm::u32 DescriptorSetLayout::Size() const
{
    return m_descriptors.size();
}

glm::u32 DescriptorSetLayoutList::Push(const DescriptorSetLayout& layout)
{
    m_descriptorSets.push_back(layout);
    return m_descriptorSets.size() - 1;
}

const std::vector<DescriptorSetLayout>& DescriptorSetLayoutList::GetDescriptorSets() const
{
    return m_descriptorSets;
}

const DescriptorSetLayout& DescriptorSetLayoutList::GetSet(glm::u32 index) const
{
    DEBUG_ASSERT(index < m_descriptorSets.size());
    return m_descriptorSets[index];
}

glm::u32 DescriptorSetLayoutList::Size() const
{
    return m_descriptorSets.size();
}
