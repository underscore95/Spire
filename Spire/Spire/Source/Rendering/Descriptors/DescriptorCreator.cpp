#include "DescriptorCreator.h"
#include "Rendering/Memory/VulkanBuffer.h"
#include "Rendering/Memory/VulkanImage.h"
#include "Rendering/Memory/PerImageBuffer.h"

namespace Spire {
    DescriptorCreator::DescriptorCreator(glm::u32 numSwapchainImages)
        : m_numSwapchainImages(numSwapchainImages) {
    }

    PerImageDescriptor DescriptorCreator::CreatePerImageUniformBuffer(
        glm::u32 binding,
        const PerImageBuffer &buffer,
        VkShaderStageFlags stages
    ) const {
        return CreatePerImageDescriptor(
            binding,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            1,
            sizeof(VulkanBuffer),
            buffer.GetBuffers().data(),
            stages
        );
    }

    PerImageDescriptor DescriptorCreator::CreatePerImageStorageBuffer(glm::u32 binding, const PerImageBuffer &buffer,
                                                                      VkShaderStageFlags stages) const {
        return CreatePerImageDescriptor(
            binding,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            1,
            sizeof(VulkanBuffer),
            buffer.GetBuffers().data(),
            stages
        );
    }

    PerImageDescriptor DescriptorCreator::CreatePerImageDescriptor(
        glm::u32 binding,
        VkDescriptorType resourceType,
        glm::u32 numResourcesPerImage,
        glm::u32 resourceSize,
        const void *resources,
        VkShaderStageFlags stages
    ) const {
        assert(resources != nullptr);

        PerImageDescriptor perImageDescriptor;
        for (glm::u32 i = 0; i < m_numSwapchainImages; i++) {
            perImageDescriptor.Descriptors.push_back(Descriptor{
                .ResourceType = resourceType,
                .Binding = binding,
                .Stages = stages,
                .Resources = {}
            });

            perImageDescriptor.Descriptors.back().Resources.reserve(numResourcesPerImage);
            const char *resourcesInThisImage = static_cast<const char *>(resources) + (resourceSize * i * numResourcesPerImage);
            for (const char *resourcePtr = resourcesInThisImage; resourcePtr < resourcesInThisImage + resourceSize * numResourcesPerImage; resourcePtr += resourceSize) {
                perImageDescriptor.Descriptors.back().Resources.push_back({resourcePtr});
            }
        }

        return perImageDescriptor;
    }
}
