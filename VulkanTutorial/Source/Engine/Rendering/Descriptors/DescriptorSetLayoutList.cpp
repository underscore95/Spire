#include "DescriptorSetLayoutList.h"
#include <libassert/assert.hpp>

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
