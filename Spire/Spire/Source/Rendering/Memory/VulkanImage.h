#pragma once

#include "pch.h"

namespace Spire {
    struct VulkanImage {
        VkImage Image = VK_NULL_HANDLE;
        VmaAllocation Allocation = nullptr;
        VkImageView ImageView = VK_NULL_HANDLE;
        VkSampler Sampler = VK_NULL_HANDLE;
#ifndef NDEBUG
        std::string DebugName;
#endif
    };
}
