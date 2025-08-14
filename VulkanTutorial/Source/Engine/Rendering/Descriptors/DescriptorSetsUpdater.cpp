#include "DescriptorSetsUpdater.h"
#include <libassert/assert.hpp>
#include "Engine/Rendering/Memory/VulkanImage.h"
#include "Engine/Rendering/Memory/VulkanBuffer.h"
#include "DescriptorSetLayout.h"

DescriptorSetsUpdater::DescriptorSetsUpdater(
    VkDevice device,
    glm::u32 numDescriptorSets,
    const DescriptorSetLayout* descriptorSets
) : m_device(device)
{
    // Constructor just prepares the write descriptor sets but doesn't update anything
    DEBUG_ASSERT(numDescriptorSets > 0);
    DEBUG_ASSERT(descriptorSets != nullptr);

    // Reserve space since we are using memory addresses so can't have resizing
    glm::u32 totalDescriptorCount = 0;
    glm::u32 totalWriteDescriptorsRequired = 0;
    for (glm::u32 i = 0; i < numDescriptorSets; i++)
    {
        const DescriptorSetLayout& descriptorSet = descriptorSets[i];
        for (const Descriptor& descriptor : descriptorSet.Descriptors)
        {
            totalWriteDescriptorsRequired++;
            totalDescriptorCount += descriptor.NumResources;
        }
    }
    m_writeDescriptorSets.reserve(totalWriteDescriptorsRequired);
    m_descriptorInfos.reserve(totalDescriptorCount);

    // Create the write descriptors
    for (glm::u32 i = 0; i < numDescriptorSets; i++)
    {
        const DescriptorSetLayout& descriptorSet = descriptorSets[i];
        for (const Descriptor& descriptor : descriptorSet.Descriptors)
        {
            DEBUG_ASSERT(descriptor.ResourcePtrs.Raw != nullptr);
            DEBUG_ASSERT(descriptor.NumResources >= 1);

            VkWriteDescriptorSet writeDescriptorSet = {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = descriptorSet.Handle,
                .dstBinding = descriptor.Binding,
                .dstArrayElement = 0,
                .descriptorCount = descriptor.NumResources,
                .descriptorType = descriptor.ResourceType,
            };

            // Descriptor infos
            glm::u32 firstDescriptorInfoIndex = m_descriptorInfos.size();

            for (glm::u32 resourceIndex = 0; resourceIndex < descriptor.NumResources; resourceIndex++)
            {
                DescriptorInfo descriptorInfo = {};

                // Buffers
                if (IsBuffer(descriptor.ResourceType))
                {
                    descriptorInfo.BufferInfo = {
                        .buffer = descriptor.ResourcePtrs.Buffers[resourceIndex].Buffer,
                        .offset = 0,
                        .range = VK_WHOLE_SIZE
                    };
                }

                // Textures
                else if (IsTextureSampler(descriptor.ResourceType))
                {
                    const VulkanImage& texture = descriptor.ResourcePtrs.Textures[resourceIndex];
                    descriptorInfo.ImageInfo = {
                        .sampler = texture.Sampler,
                        .imageView = texture.ImageView,
                        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                    };
                }

                // Unsupported resource type
                else
                    UNREACHABLE();

                m_descriptorInfos.push_back(descriptorInfo);
            }

            // Point to first descriptor info
            if (IsBuffer(descriptor.ResourceType))
            {
                writeDescriptorSet.pBufferInfo = &(m_descriptorInfos[firstDescriptorInfoIndex].BufferInfo);
            }

            else if (IsTextureSampler(descriptor.ResourceType))
            {
                writeDescriptorSet.pImageInfo = &(m_descriptorInfos[firstDescriptorInfoIndex].ImageInfo);
            }

            // Unsupported resource type
            else
                UNREACHABLE();

            m_writeDescriptorSets.push_back(writeDescriptorSet);
        }
    }

    DEBUG_ASSERT(m_writeDescriptorSets.size() == totalWriteDescriptorsRequired);
    DEBUG_ASSERT(m_descriptorInfos.size() == totalDescriptorCount);
}

DescriptorSetsUpdater::~DescriptorSetsUpdater()
{
    DEBUG_ASSERT(m_hasUpdatedAtLeastOnce);
    // You haven't done anything with this you need to call Update() to actually update the descriptors
}

void DescriptorSetsUpdater::Update()
{
    DEBUG_ASSERT(!m_writeDescriptorSets.empty());
    m_hasUpdatedAtLeastOnce = true;

    vkUpdateDescriptorSets(m_device, m_writeDescriptorSets.size(), m_writeDescriptorSets.data(), 0, nullptr);
}

bool DescriptorSetsUpdater::IsBuffer(VkDescriptorType resourceType) const
{
    switch (resourceType)
    {
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        return true;
    default:
        return false;
    }
}

bool DescriptorSetsUpdater::IsTextureSampler(VkDescriptorType resourceType) const
{
    switch (resourceType)
    {
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        return true;
    default:
        return false;
    }
}

bool DescriptorSetsUpdater::IsSupportedResourceType(VkDescriptorType resourceType) const
{
    return IsBuffer(resourceType) || IsTextureSampler(resourceType);
}
