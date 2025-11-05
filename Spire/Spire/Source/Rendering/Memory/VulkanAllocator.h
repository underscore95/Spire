#pragma once

#include "pch.h"
#include "Rendering/Core/VulkanVersion.h"

namespace Spire
{
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
}