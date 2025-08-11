#pragma once

#include <vk_mem_alloc.h>
#include "Engine/Rendering/Core/VulkanVersion.h"

class VulkanAllocator
{
public:
    VulkanAllocator(VkDevice device, VkPhysicalDevice physicalDevice, VkInstance instance,
                    const VulkanVersion& version);
    ~VulkanAllocator();

public:
    [[nodiscard]] VmaAllocator GetAllocator() const;

private:
    VmaAllocator m_allocator;
};
