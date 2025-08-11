#pragma once
#include <glm/fwd.hpp>
#include <vulkan/vulkan_core.h>

struct VulkanImage;
class VulkanBuffer;
class RenderingManager;

struct Descriptor
{
    VkDescriptorType ResourceType;
    glm::u32 Binding;
    VkShaderStageFlags Stages;

    glm::u32 NumResources;
    union ResourcePtr
    {
        const void* Raw;
        const VulkanBuffer* Buffers;
        const VulkanImage* Textures;
    } ResourcePtrs;
};