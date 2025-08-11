#include "DescriptorSet.h"

#include <unordered_set>
#include <libassert/assert.hpp>
#include <spdlog/spdlog.h>

DescriptorSet::DescriptorSet(
    VkDevice device,
    const std::vector<Descriptor>& descriptors
) : m_device(device),
    m_descriptors(descriptors)
{
    GenerateLayout();
}

DescriptorSet::~DescriptorSet()
{
    vkDestroyDescriptorSetLayout(m_device, m_layout, nullptr);
}

std::vector<Descriptor>::const_iterator DescriptorSet::begin() const
{
    return m_descriptors.begin();
}

std::vector<Descriptor>::const_iterator DescriptorSet::end() const
{
    return m_descriptors.end();
}

void DescriptorSet::GenerateLayout()
{
    std::unordered_set<glm::u32> usedBindings;
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings;

    for (const auto& descriptor : m_descriptors)
    {
        // check bindings are unique
        DEBUG_ASSERT(!usedBindings.contains(descriptor.Binding));
        usedBindings.insert(descriptor.Binding);

        // create layout
        layoutBindings.push_back({
            .binding = descriptor.Binding,
            .descriptorType = descriptor.ResourceType,
            .descriptorCount = descriptor.NumResources,
            .stageFlags = descriptor.Stages,
        });
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0, // reserved - must be zero
        .bindingCount = static_cast<glm::u32>(layoutBindings.size()),
        .pBindings = layoutBindings.data(),
    };

    VkResult res = vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &m_layout);
    if (res != VK_SUCCESS)
    {
        spdlog::error("Failed to create descriptor set layout");
    }
}
