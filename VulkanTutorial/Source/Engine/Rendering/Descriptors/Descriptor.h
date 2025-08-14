#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

struct VulkanImage;
class VulkanBuffer;

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
        const VulkanImage* Textures;
    } ResourcePtrs;
};