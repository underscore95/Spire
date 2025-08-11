#pragma once

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>

struct VulkanImage
{
    VkImage Image = VK_NULL_HANDLE;
    VmaAllocation Allocation = nullptr;
    VkImageView ImageView = VK_NULL_HANDLE;
    VkSampler Sampler = VK_NULL_HANDLE;
};
