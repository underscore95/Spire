#include "Engine/Rendering/Memory/VulkanAllocator.h"
#include "Utils/Log.h"

namespace Spire
{
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
            error("Error creating vulkan memory allocator");
        }
        else
        {
            info("Created vulkan memory allocator");
        }
    }

    VulkanAllocator::~VulkanAllocator()
    {
        vmaDestroyAllocator(m_allocator);
        info("Destroyed vulkan memory allocator");
    }

    VmaAllocator VulkanAllocator::GetAllocator() const
    {
        return m_allocator;
    }
}