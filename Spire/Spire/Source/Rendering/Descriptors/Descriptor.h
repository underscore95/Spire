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

        union ResourcePtr
        {
            const void* Raw;
            const VulkanBuffer* Buffer;
            const VulkanImage* Image;
        } ;
        std::vector<ResourcePtr> Resources; // This struct can represent multiple descriptors if it is e.g. an array of images

#ifndef NDEBUG
    std::string DebugName;
#endif
    };

    struct PerImageDescriptor
    {
        std::vector<Descriptor> Descriptors; // each descriptor should go in a different set
    };
}