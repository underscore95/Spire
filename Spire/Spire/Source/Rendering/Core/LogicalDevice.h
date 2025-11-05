#pragma once

#include "pch.h"

#include "VulkanVersion.h"

namespace Spire
{
    class RenderingDeviceManager;

    class LogicalDevice
    {
    public:
        LogicalDevice(
            RenderingDeviceManager& deviceManager,
            VulkanVersion vulkanVersion
        );
        ~LogicalDevice();

    public:
        [[nodiscard]] bool IsValid() const;
        [[nodiscard]] VkDevice GetDevice() const;
        [[nodiscard]] glm::u32 GetDeviceQueueFamily() const;

    private:
        enum class DynamicRenderingSupport
        {
            NONE, EXTENSION_REQUIRED, SUPPORTED
        };

        [[nodiscard]] DynamicRenderingSupport GetDynamicRenderingSupport(const RenderingDeviceManager& deviceManager,
                                                                         const VulkanVersion& vulkanVersion) const;

    private:
        const glm::u32 INVALID_DEVICE_QUEUE_FAMILY = -1; // underflow
        glm::u32 m_deviceQueueFamily = INVALID_DEVICE_QUEUE_FAMILY;
        VkDevice m_device = VK_NULL_HANDLE;
    };
}