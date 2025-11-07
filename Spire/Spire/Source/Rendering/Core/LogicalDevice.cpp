#include "LogicalDevice.h"
#include "Utils/Log.h"
#include "RenderingDeviceManager.h"

namespace Spire {
    LogicalDevice::LogicalDevice(
        RenderingDeviceManager &deviceManager,
        VulkanVersion vulkanVersion) {
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
        else if (dynamicRenderingSupport == DynamicRenderingSupport::NONE) {
            error("Dynamic rendering is not supported.");
            return;
        }

        if (vulkanVersion < VulkanVersion{.RawVersion = VK_API_VERSION_1_2}) {
            deviceExtensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
        }

        if (deviceManager.Selected().Features.geometryShader == VK_FALSE) {
            error("The Geometry Shader is not supported!");
            return;
        }

        if (deviceManager.Selected().Features.tessellationShader == VK_FALSE) {
            error("The Tessellation Shader is not supported!");
            return;
        }

        if (!SupportsDescriptorIndexing(deviceManager)) {
            error("Descriptor indexing not supported on this device");
            return;
        }

        // dynamic rendering
        VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeature = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
            .pNext = nullptr,
            .dynamicRendering = VK_TRUE
        };

        // indexing
        VkPhysicalDeviceDescriptorIndexingFeaturesEXT indexingFeatures = {};
        indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
        indexingFeatures.pNext = &dynamicRenderingFeature;
        indexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
        indexingFeatures.shaderStorageBufferArrayNonUniformIndexing = VK_TRUE;
        indexingFeatures.runtimeDescriptorArray = VK_TRUE;
        indexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;

        VkPhysicalDeviceFeatures2 deviceFeatures2 = {};
        deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        deviceFeatures2.pNext = &indexingFeatures;

        // device features
        deviceFeatures2.features.geometryShader = VK_TRUE;
        deviceFeatures2.features.tessellationShader = VK_TRUE;

        VkDeviceCreateInfo deviceCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = &deviceFeatures2,
            .flags = 0,
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &queueInfo,
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = nullptr,
            .enabledExtensionCount = static_cast<glm::u32>(deviceExtensions.size()),
            .ppEnabledExtensionNames = deviceExtensions.data(),
            .pEnabledFeatures = nullptr
        };

        // create
        VkResult result = vkCreateDevice(
            deviceManager.Selected().PhysicalDeviceHandle,
            &deviceCreateInfo, nullptr,
            &m_device
        );
        if (result != VK_SUCCESS) {
            error("Failed to create logical device");
        } else {
            info("Created logical device");
        }
    }

    LogicalDevice::~LogicalDevice() {
        if (m_device) {
            vkDestroyDevice(m_device, nullptr);
            info("Destroyed logical device");
        }
    }

    bool LogicalDevice::IsValid() const {
        return m_deviceQueueFamily != INVALID_DEVICE_QUEUE_FAMILY && m_device;
    }

    VkDevice LogicalDevice::GetDevice() const {
        return m_device;
    }

    glm::u32 LogicalDevice::GetDeviceQueueFamily() const {
        return m_deviceQueueFamily;
    }

    LogicalDevice::DynamicRenderingSupport LogicalDevice::GetDynamicRenderingSupport(
        const RenderingDeviceManager &deviceManager, const VulkanVersion &vulkanVersion) {
        bool dynamicRenderingExtension = deviceManager.IsExtensionSupported(
            deviceManager.Selected(), VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        bool instanceVersionGreaterThanOrEqualTo_0_1_3_0 = (vulkanVersion.Variant > 0) || (vulkanVersion.Major > 1)
                                                           || (vulkanVersion.Minor >= 3);

        if (instanceVersionGreaterThanOrEqualTo_0_1_3_0) {
            if (!dynamicRenderingExtension)
                warn(
                    "Vulkan instance is >= 0.1.3.0 but didn't have the dynamic rendering extension, this probably isn't an issue unless your vulkan imploded and this warning popped up.");
            return DynamicRenderingSupport::SUPPORTED;
        }

        return dynamicRenderingExtension ? DynamicRenderingSupport::EXTENSION_REQUIRED : DynamicRenderingSupport::NONE;
    }

    bool LogicalDevice::SupportsDescriptorIndexing(const RenderingDeviceManager &deviceManager) {
        VkPhysicalDeviceDescriptorIndexingFeaturesEXT indexingFeatures = {};
        indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
        indexingFeatures.pNext = nullptr;

        VkPhysicalDeviceFeatures2 deviceFeatures2 = {};
        deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        deviceFeatures2.pNext = &indexingFeatures;

        vkGetPhysicalDeviceFeatures2(deviceManager.Selected().PhysicalDeviceHandle, &deviceFeatures2);
        return indexingFeatures.shaderStorageBufferArrayNonUniformIndexing && indexingFeatures.runtimeDescriptorArray;
    }
}
