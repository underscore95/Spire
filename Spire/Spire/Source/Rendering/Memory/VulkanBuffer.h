#pragma once

#include "pch.h"

namespace Spire {
    struct VulkanBuffer {
    public:
        VkBuffer Buffer = VK_NULL_HANDLE;
        VmaAllocation Allocation = nullptr;
        std::size_t Size = 0; // size in bytes
        glm::u32 Count = 0; // number of elements
        glm::u32 ElementSize = 0; // size of a single element in bytes
    };
}
