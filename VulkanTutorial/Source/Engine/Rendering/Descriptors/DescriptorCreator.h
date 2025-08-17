#pragma once

#include <glm/glm.hpp>
#include "Descriptor.h"
#include "Engine/Rendering/Memory/PerImageBuffer.h"

namespace Spire
{
    struct PerImageDescriptor;
    struct VulkanBuffer;

    // Helper class to fill out Descriptor and PerImageDescriptor structs, you can manually fill them out if it is easier
    class DescriptorCreator
    {
    public:
        explicit DescriptorCreator(glm::u32 numSwapchainImages);

    public:
        PerImageDescriptor CreatePerImageUniformBuffer(
            glm::u32 binding,
            const PerImageBuffer& buffer,
            VkShaderStageFlags stages = VK_SHADER_STAGE_ALL
        ) const;

        // Resources should be laid out like this
        // assuming 2 swapchain images and buffers A1A2A3 for image 1 and B1B2B3 for image 2
        // numResourcesPerImage would be 3 and resources would be A1A2A3B1B2B3
        // more than 1 resource per image means it is an array in the shader!
        // resourceSize is the size of the resource, if you are giving it an array of VulkanBuffer then it would be sizeof(VulkanBuffer)
        // resources type must be a valid type in the ResourcePtrs union of Descriptor
        // you must keep the resources alive until the descriptor has been destroyed!
        PerImageDescriptor CreatePerImageDescriptor(
            glm::u32 binding,
            VkDescriptorType resourceType,
            glm::u32 numResourcesPerImage,
            glm::u32 resourceSize,
            const void* resources,
            VkShaderStageFlags stages = VK_SHADER_STAGE_ALL
        ) const;

    private:
        glm::u32 m_numSwapchainImages;
    };
}