#include "DescriptorPool.h"
#include <glm/glm.hpp>
#include <unordered_map>
#include <libassert/assert.hpp>
#include <spdlog/spdlog.h>
#include "Descriptor.h"
#include "DescriptorSet.h"
#include "DescriptorSetLayout.h"
#include "Engine/Rendering/RenderingManager.h"

DescriptorPool::DescriptorPool(
    RenderingManager& renderingManager,
    const std::vector<DescriptorSetLayout>& descriptorSets)
    : m_renderingManager(renderingManager)
{
    // Count number of each type of resource
    std::unordered_map<VkDescriptorType, glm::u32> numResources;
    for (const auto& descriptorSet : descriptorSets)
    {
        for (const auto& descriptor : descriptorSet)
        {
            numResources[descriptor.ResourceType] += descriptor.NumResources;
        }
    }

    // Generate pool sizes
    std::vector<VkDescriptorPoolSize> sizes;
    for (auto& [descriptorType, descriptorCount] : numResources)
    {
        sizes.push_back({
            .type = descriptorType,
            .descriptorCount = descriptorCount
        });
    }

    // Create pool
    VkDescriptorPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = 0,
        .maxSets = static_cast<glm::u32>(descriptorSets.size()),
        .poolSizeCount = static_cast<glm::u32>(sizes.size()),
        .pPoolSizes = sizes.data()
    };

    VkResult res = vkCreateDescriptorPool(renderingManager.GetDevice(), &poolInfo, nullptr, &m_descriptorPool);
    if (res != VK_SUCCESS)
    {
        spdlog::error("Failed to create descriptor pool");
    }
    else
    {
        spdlog::info("Created descriptor pool");
    }
}

DescriptorPool::~DescriptorPool()
{
    DEBUG_ASSERT(m_numAllocatedSets == 0);

    vkDestroyDescriptorPool(m_renderingManager.GetDevice(), m_descriptorPool, nullptr);
}

std::vector<DescriptorSet> DescriptorPool::Allocate(
    glm::u32 numDescriptorSets,
    const DescriptorSetLayout* descriptorSets
)
{
    // Create vector of layouts
    std::vector<VkDescriptorSetLayout> layouts;
    layouts.reserve(numDescriptorSets);
    for (glm::u32 i = 0; i < numDescriptorSets; i++)
    {
        layouts.push_back(descriptorSets[i].GetLayout());
    }

    // Make allocation
    std::vector<VkDescriptorSet> descriptorSets;
    descriptorSets.resize(numDescriptorSets);

    VkDescriptorSetAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = m_descriptorPool,
        .descriptorSetCount = numDescriptorSets,
        .pSetLayouts = layouts.data()
    };

    VkResult res = vkAllocateDescriptorSets(m_renderingManager.GetDevice(), &allocInfo, descriptorSets.data());

    if (res != VK_SUCCESS)
    {
        spdlog::error("Failed to allocate descriptor sets");
    }
    else m_numAllocatedSets += numDescriptorSets;

    // Convert to our wrapper
    std::vector<DescriptorSet> sets;
    sets.reserve(numDescriptorSets);
    for (VkDescriptorSet set : descriptorSets)
    {
        sets.push_back(DescriptorSet{set});
    }
    return sets;
}

void DescriptorPool::Free(glm::u32 numDescriptorSets, const DescriptorSet* descriptorSets)
{
    DEBUG_ASSERT(m_numAllocatedSets > numDescriptorSets);
    DEBUG_ASSERT(descriptorSets != nullptr);

    // Convert from our wrapper to vulkan handles
    std::vector<VkDescriptorSet> vulkanDescriptorSets;
    vulkanDescriptorSets.resize(numDescriptorSets);
    for (glm::u32 i = 0; i < numDescriptorSets; i++)
    {
        vulkanDescriptorSets[i] = descriptorSets[i].GetHandle();
    }

    // Free
    VkResult res = vkFreeDescriptorSets(
        m_renderingManager.GetDevice(),
        m_descriptorPool,
        vulkanDescriptorSets.size(),
        vulkanDescriptorSets.data()
    );

    if (res != VK_SUCCESS)
    {
        spdlog::error("Failed to free descriptor sets");
    }
    else m_numAllocatedSets -= numDescriptorSets;
}
