#pragma once

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>

struct VulkanBuffer
{
public:
    VkBuffer Buffer = VK_NULL_HANDLE;
    VmaAllocation Allocation = nullptr;
};
