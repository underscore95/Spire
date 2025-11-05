#pragma once

#include "pch.h"

namespace Spire
{
    struct VulkanImage;
    struct VulkanBuffer;

    struct Descriptor
    {
        VkDescriptorType ResourceType;
        glm::u32 Binding;
        VkShaderStageFlags Stages;

        glm::u32 NumResources; // This struct can represent multiple descriptors if it is e.g. an array of images
        union ResourcePtr
        {
            const void* Raw;
            const VulkanBuffer* Buffers;
            const VulkanImage* Images;
        } ResourcePtrs;
    };

    struct PerImageDescriptor
    {
        std::vector<Descriptor> Descriptors; // each descriptor should go in a different set
    };
}