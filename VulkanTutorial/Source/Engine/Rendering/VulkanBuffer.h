#pragma once

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>

struct VulkanBuffer
{
public:
    VkBuffer Buffer = VK_NULL_HANDLE;
    VmaAllocation Allocation = nullptr;
    glm::u32 Size = 0; // size in bytes
    glm::u32 Count = 0; // number of elements
    glm::u32 ElementSize = 0; // size of a single element in bytes
};