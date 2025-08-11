#include "DescriptorSet.h"

DescriptorSet::DescriptorSet(VkDescriptorSet descriptorSet)
    :m_descriptorSet(descriptorSet)
{
}

DescriptorSet::~DescriptorSet()
{
}

VkDescriptorSet DescriptorSet::GetHandle() const
{
    return m_descriptorSet;
}
