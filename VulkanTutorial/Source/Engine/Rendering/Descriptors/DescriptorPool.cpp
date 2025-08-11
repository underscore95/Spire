#include "DescriptorPool.h"
#include <glm/glm.hpp>
#include <unordered_map>
#include <spdlog/spdlog.h>
#include "Descriptor.h"
#include "DescriptorSet.h"
#include "Engine/Rendering/RenderingManager.h"

DescriptorPool::DescriptorPool(
    RenderingManager& renderingManager,
    const std::vector<DescriptorSet>& descriptorSets)
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
    vkDestroyDescriptorPool(m_renderingManager.GetDevice(), m_descriptorPool, nullptr);
}
