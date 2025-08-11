#include "Engine/Rendering/Memory/VulkanAllocator.h"
#include <spdlog/spdlog.h>

VulkanAllocator::VulkanAllocator(VkDevice device, VkPhysicalDevice physicalDevice, VkInstance instance,
                                 const VulkanVersion& version)
{
    VmaAllocatorCreateInfo createInfo = {};
    createInfo.device = device;
    createInfo.physicalDevice = physicalDevice;
    createInfo.instance = instance;

    // 1.3 is latest supported by vma
    createInfo.vulkanApiVersion = glm::min(version.RawVersion, VK_MAKE_API_VERSION(0, 1, 3, 0));

    VkResult res = vmaCreateAllocator(&createInfo, &m_allocator);

    if (res != VK_SUCCESS)
    {
        spdlog::error("Error creating vulkan memory allocator");
    }
    else
    {
        spdlog::info("Created vulkan memory allocator");
    }
}

VulkanAllocator::~VulkanAllocator()
{
    vmaDestroyAllocator(m_allocator);
    spdlog::info("Destroyed vulkan memory allocator");
}

VmaAllocator VulkanAllocator::GetAllocator() const
{
    return m_allocator;
}
