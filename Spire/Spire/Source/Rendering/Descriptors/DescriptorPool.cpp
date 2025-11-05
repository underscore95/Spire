#include "DescriptorPool.h"
#include <glm/glm.hpp>
#include <unordered_map>
#include <unordered_set>

#include "Utils/Log.h"
#include "Descriptor.h"
#include "DescriptorSet.h"
#include "DescriptorSetLayoutList.h"
#include "Rendering/RenderingManager.h"

namespace Spire
{
    DescriptorPool::DescriptorPool(
        RenderingManager& renderingManager)
        : m_renderingManager(renderingManager)
    {
        // Generate pool sizes
        std::vector<VkDescriptorPoolSize> sizes;

        sizes.push_back({
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 100
        });
        sizes.push_back({
            .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 100
        });
        sizes.push_back({
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 100
        });

        // Create pool
        VkDescriptorPoolCreateInfo poolInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
            .maxSets = static_cast<glm::u32>(300),
            .poolSizeCount = static_cast<glm::u32>(sizes.size()),
            .pPoolSizes = sizes.data()
        };

        VkResult res = vkCreateDescriptorPool(renderingManager.GetDevice(), &poolInfo, nullptr, &m_descriptorPool);
        if (res != VK_SUCCESS)
        {
            error("Failed to create descriptor pool");
        }
        else
        {
            info("Created descriptor pool");
        }
    }

    DescriptorPool::~DescriptorPool()
    {
        assert(m_numAllocatedSets == 0);

        vkDestroyDescriptorPool(m_renderingManager.GetDevice(), m_descriptorPool, nullptr);
    }

    std::vector<DescriptorSet> DescriptorPool::Allocate(
        const DescriptorSetLayoutList& descriptorsLists
    )
    {
        // Generate layouts
        std::vector<VkDescriptorSetLayout> layouts;
        layouts.reserve(descriptorsLists.Size());

        for (const auto& descriptors : descriptorsLists.GetDescriptorSets())
        {
            std::unordered_set<glm::u32> usedBindings;
            std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
            layoutBindings.reserve(descriptors.size());

            for (const auto& descriptor : descriptors)
            {
                // check bindings are unique
                assert(!usedBindings.contains(descriptor.Binding));
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

            VkDescriptorSetLayout layout = VK_NULL_HANDLE;
            VkResult res = vkCreateDescriptorSetLayout(m_renderingManager.GetDevice(), &layoutInfo, nullptr, &layout);
            layouts.push_back(layout);
            if (res != VK_SUCCESS)
            {
                error("Failed to create descriptor set layout");
            }
        }

        // Make allocation
        std::vector<VkDescriptorSet> rawDescriptorSets;
        rawDescriptorSets.resize(descriptorsLists.Size());

        VkDescriptorSetAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = nullptr,
            .descriptorPool = m_descriptorPool,
            .descriptorSetCount = descriptorsLists.Size(),
            .pSetLayouts = layouts.data()
        };

        VkResult res = vkAllocateDescriptorSets(m_renderingManager.GetDevice(), &allocInfo, rawDescriptorSets.data());

        if (res != VK_SUCCESS)
        {
            error("Failed to allocate descriptor sets");
        }
        else m_numAllocatedSets += descriptorsLists.Size();

        // Convert to our wrapper
        std::vector<DescriptorSet> descriptorSets;
        descriptorSets.reserve(rawDescriptorSets.size());
        for (int i = 0; i < rawDescriptorSets.size(); i++)
        {
            descriptorSets.push_back(DescriptorSet{
                .Descriptors = descriptorsLists.GetSet(i),
                .Layout = layouts[i],
                .Handle = rawDescriptorSets[i]
            });
        }
        return descriptorSets;
    }

    void DescriptorPool::Free(const std::vector<DescriptorSet>& descriptorSets)
    {
        assert(m_numAllocatedSets >= descriptorSets.size());
        assert(!descriptorSets.empty());

        // Convert from our wrapper to vulkan handles
        std::vector<VkDescriptorSet> vulkanDescriptorSets;
        vulkanDescriptorSets.resize(descriptorSets.size());
        for (glm::u32 i = 0; i < descriptorSets.size(); i++)
        {
            vulkanDescriptorSets[i] = descriptorSets[i].Handle;
        }

        // Free
        VkResult res = vkFreeDescriptorSets(
            m_renderingManager.GetDevice(),
            m_descriptorPool,
            vulkanDescriptorSets.size(),
            vulkanDescriptorSets.data()
        );
        for (int i = 0; i < descriptorSets.size(); i++)
        {
            vkDestroyDescriptorSetLayout(m_renderingManager.GetDevice(), descriptorSets[i].Layout, nullptr);
        }

        if (res != VK_SUCCESS)
        {
            error("Failed to free descriptor sets");
        }
        else m_numAllocatedSets -= descriptorSets.size();
    }
}