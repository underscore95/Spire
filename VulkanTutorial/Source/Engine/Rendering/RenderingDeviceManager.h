#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>
#include <glm/fwd.hpp>

struct PhysicalDevice
{
    VkPhysicalDevice PhysicalDeviceHandle;
    VkPhysicalDeviceProperties DeviceProperties;
    std::vector<VkQueueFamilyProperties> QueueFamilyProperties; // Similar to command lists
    std::vector<VkBool32> QueueSupportsPresent;
    std::vector<VkSurfaceFormatKHR> SurfaceFormats; // Texture formats & color spaces
    VkSurfaceCapabilitiesKHR SurfaceCapabilities;
    VkPhysicalDeviceMemoryProperties MemoryProperties;
    std::vector<VkPresentModeKHR> PresentModes;
    VkPhysicalDeviceFeatures Features;
    VkFormat DepthFormat;
};

class RenderingDeviceManager
{
public:
    RenderingDeviceManager(const VkInstance& instance, const VkSurfaceKHR& surface, bool logDeviceInfo);

    ~RenderingDeviceManager();

public:
    [[nodiscard]] bool IsValid() const;

    // Select a device and return the queue family index
    [[nodiscard]] glm::u32 SelectDevice(VkQueueFlags requiredQueueType, bool supportsPresent);

    // Get the selected device
    [[nodiscard]] const PhysicalDevice& Selected() const;

private:
  [[nodiscard]]  VkFormat FindDepthFormat(VkPhysicalDevice device) const;
  [[nodiscard]]  VkFormat FindSupportedFormat(VkPhysicalDevice device, const std::vector<VkFormat>& candidates, VkImageTiling tiling,
                                 VkFormatFeatureFlags requestedFeatures) const;

private:
    int m_deviceIndex = -1;
    bool m_initialized = false;
    std::vector<PhysicalDevice> m_devices;
};
