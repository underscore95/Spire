#include "DescriptorCreator.h"
#include "Engine/Rendering/Memory/VulkanBuffer.h"
#include "Engine/Rendering/Memory/VulkanImage.h"
#include <libassert/assert.hpp>

#include "Engine/Rendering/Memory/PerImageBuffer.h"

namespace Spire
{
    DescriptorCreator::DescriptorCreator(glm::u32 numSwapchainImages)
        : m_numSwapchainImages(numSwapchainImages)
    {
    }

    PerImageDescriptor DescriptorCreator::CreatePerImageUniformBuffer(
        glm::u32 binding,
        const PerImageBuffer& buffer,
        VkShaderStageFlags stages
    ) const
    {
        return CreatePerImageDescriptor(
            binding,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            1,
            sizeof(VulkanBuffer),
            buffer.GetBuffers().data(),
            stages);
    }

    PerImageDescriptor DescriptorCreator::CreatePerImageDescriptor(
        glm::u32 binding,
        VkDescriptorType resourceType,
        glm::u32 numResourcesPerImage,
        glm::u32 resourceSize,
        const void* resources,
        VkShaderStageFlags stages
    ) const
    {
        DEBUG_ASSERT(resources != nullptr);

        PerImageDescriptor perImageDescriptor;
        for (glm::u32 i = 0; i < m_numSwapchainImages; i++)
        {
            perImageDescriptor.Descriptors.push_back(Descriptor{
                .ResourceType = resourceType,
                .Binding = binding,
                .Stages = stages,
                .NumResources = numResourcesPerImage,
                .ResourcePtrs = static_cast<const char*>(resources) + (resourceSize * i * numResourcesPerImage),
            });
        }

        return perImageDescriptor;
    }
}