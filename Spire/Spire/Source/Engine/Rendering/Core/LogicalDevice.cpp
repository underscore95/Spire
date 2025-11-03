#include "LogicalDevice.h"
#include <spdlog/spdlog.h>
#include "RenderingDeviceManager.h"

namespace Spire
{
    LogicalDevice::LogicalDevice(
        RenderingDeviceManager& deviceManager,
        VulkanVersion vulkanVersion)
    {
        m_deviceQueueFamily = deviceManager.SelectDevice(VK_QUEUE_GRAPHICS_BIT, true);

        float queuePriorities[] = {1.0f};

        VkDeviceQueueCreateInfo queueInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0, // must be zero
            .queueFamilyIndex = m_deviceQueueFamily,
            .queueCount = 1,
            .pQueuePriorities = &queuePriorities[0]
        };

        std::vector deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME
        };

        DynamicRenderingSupport dynamicRenderingSupport = GetDynamicRenderingSupport(deviceManager, vulkanVersion);
        if (dynamicRenderingSupport == DynamicRenderingSupport::EXTENSION_REQUIRED)
            deviceExtensions.push_back(
                VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        else if (dynamicRenderingSupport == DynamicRenderingSupport::NONE)
        {
            spdlog::error("Dynamic rendering is not supported.");
            return;
        }

        if (deviceManager.Selected().Features.geometryShader == VK_FALSE)
        {
            spdlog::error("The Geometry Shader is not supported!");
            return;
        }

        if (deviceManager.Selected().Features.tessellationShader == VK_FALSE)
        {
            spdlog::error("The Tessellation Shader is not supported!");
            return;
        }

        VkPhysicalDeviceFeatures deviceFeatures = {};
        deviceFeatures.geometryShader = VK_TRUE;
        deviceFeatures.tessellationShader = VK_TRUE;

        VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
            .pNext = nullptr,
            .dynamicRendering = VK_TRUE
        };

        VkDeviceCreateInfo deviceCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = &dynamicRenderingFeature,
            .flags = 0,
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &queueInfo,
            .enabledLayerCount = 0, // DEPRECATED
            .ppEnabledLayerNames = nullptr, // DEPRECATED
            .enabledExtensionCount = static_cast<glm::u32>(deviceExtensions.size()),
            .ppEnabledExtensionNames = deviceExtensions.data(),
            .pEnabledFeatures = &deviceFeatures
        };

        VkResult result = vkCreateDevice(
            deviceManager.Selected().PhysicalDeviceHandle,
            &deviceCreateInfo, nullptr,
            &m_device
        );
        if (result != VK_SUCCESS)
        {
            spdlog::error("Failed to create logical device");
        }
        else
        {
            spdlog::info("Created logical device");
        }
    }

    LogicalDevice::~LogicalDevice()
    {
        if (m_device)
        {
            vkDestroyDevice(m_device, nullptr);
            spdlog::info("Destroyed logical device");
        }
    }

    bool LogicalDevice::IsValid() const
    {
        return m_deviceQueueFamily != INVALID_DEVICE_QUEUE_FAMILY && m_device;
    }

    VkDevice LogicalDevice::GetDevice() const
    {
        return m_device;
    }

    glm::u32 LogicalDevice::GetDeviceQueueFamily() const
    {
        return m_deviceQueueFamily;
    }

    LogicalDevice::DynamicRenderingSupport LogicalDevice::GetDynamicRenderingSupport(
        const RenderingDeviceManager& deviceManager, const VulkanVersion& vulkanVersion) const
    {
        bool dynamicRenderingExtension = deviceManager.IsExtensionSupported(
            deviceManager.Selected(), VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        bool instanceVersionGreaterThanOrEqualTo_0_1_3_0 = (vulkanVersion.Variant > 0) || (vulkanVersion.Major > 1)
            || (vulkanVersion.Minor >= 3);

        if (instanceVersionGreaterThanOrEqualTo_0_1_3_0)
        {
            if (!dynamicRenderingExtension)
                spdlog::warn(
                    "Vulkan instance is >= 0.1.3.0 but didn't have the dynamic rendering extension, this probably isn't an issue unless your vulkan imploded and this warning popped up.");
            return DynamicRenderingSupport::SUPPORTED;
        }

        return dynamicRenderingExtension ? DynamicRenderingSupport::EXTENSION_REQUIRED : DynamicRenderingSupport::NONE;
    }
}