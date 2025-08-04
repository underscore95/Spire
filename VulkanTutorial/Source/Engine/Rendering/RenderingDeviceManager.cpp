#include "RenderingDeviceManager.h"
#include <glm/glm.hpp>
#include <libassert/assert.hpp>
#include <spdlog/spdlog.h>

RenderingDeviceManager::RenderingDeviceManager(
    const VkInstance& instance,
    const VkSurfaceKHR& surface,
    bool logDeviceInfo
)
{
    spdlog::info("Initializing RenderingDeviceManager...");
    glm::u32 numDevices = 0;

    // Get number of physical devices
    VkResult res = vkEnumeratePhysicalDevices(instance, &numDevices, nullptr);
    if (res != VK_SUCCESS)
    {
        spdlog::error("Failed to enumerate vulkan physical devices");
        return;
    }

    if (logDeviceInfo) spdlog::info("Found {} physical devices", numDevices);

    m_devices.resize(numDevices);

    std::vector<VkPhysicalDevice> physicalDevices;
    physicalDevices.resize(numDevices);

    // Get the physical devices
    res = vkEnumeratePhysicalDevices(instance, &numDevices, physicalDevices.data());
    if (res != VK_SUCCESS)
    {
        spdlog::error("Failed to enumerate vulkan physical devices ({} known physical devices)", numDevices);
        return;
    }

    // Get information about each device
    for (glm::u32 i = 0; i < numDevices; i++)
    {
        std::string logMessage = std::format("Device #{}\n", i);
        logMessage.reserve(128);

        // Device
        VkPhysicalDevice physicalDevice = physicalDevices[i];
        m_devices[i].PhysicalDeviceHandle = physicalDevice;

        // Device properties
        vkGetPhysicalDeviceProperties(physicalDevice, &m_devices[i].DeviceProperties);

        logMessage += std::format("    Name: {}\n", m_devices[i].DeviceProperties.deviceName);

        m_devices[i].Extensions = GetExtensions(physicalDevice);
        logMessage += "    Extensions:\n";
        for (glm::u32 j = 0; j < m_devices[i].Extensions.size(); j++)
        {
            VkExtensionProperties extension = m_devices[i].Extensions[j];
            logMessage += std::format("     - {}\n", extension.extensionName);
        }

        glm::u32 apiVer = m_devices[i].DeviceProperties.apiVersion;
        logMessage += std::format(
            "    API version: {}.{}.{}.{}\n",
            VK_API_VERSION_VARIANT(apiVer),
            VK_API_VERSION_MAJOR(apiVer),
            VK_API_VERSION_MINOR(apiVer),
            VK_API_VERSION_PATCH(apiVer)
        );

        // Queue families
        glm::u32 numberQueueFamilies = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &numberQueueFamilies, nullptr);
        logMessage += std::format("    Number of family queues: {}\n", numberQueueFamilies);

        m_devices[i].QueueFamilyProperties.resize(numberQueueFamilies);
        m_devices[i].QueueSupportsPresent.resize(numberQueueFamilies);

        // Queue family properties
        vkGetPhysicalDeviceQueueFamilyProperties(
            physicalDevice,
            &numberQueueFamilies,
            m_devices[i].QueueFamilyProperties.data()
        );

        for (glm::u32 q = 0; q < numberQueueFamilies; q++)
        {
            const VkQueueFamilyProperties& queueFamilyProperties = m_devices[i].QueueFamilyProperties[q];

            logMessage += std::format("    Family {} Num queues: {} ", q, queueFamilyProperties.queueCount);
            VkQueueFlags Flags = queueFamilyProperties.queueFlags;
            logMessage += std::format("    GFX {}, Compute {}, Transfer {}, Sparse binding {}\n",
                                      (Flags & VK_QUEUE_GRAPHICS_BIT) ? "Yes" : "No",
                                      (Flags & VK_QUEUE_COMPUTE_BIT) ? "Yes" : "No",
                                      (Flags & VK_QUEUE_TRANSFER_BIT) ? "Yes" : "No",
                                      (Flags & VK_QUEUE_SPARSE_BINDING_BIT) ? "Yes" : "No");

            // Queue supports present
            res = vkGetPhysicalDeviceSurfaceSupportKHR(
                physicalDevice,
                q,
                surface,
                &(m_devices[i].QueueSupportsPresent[q])
            );

            if (res != VK_SUCCESS)
            {
                spdlog::error("Failed to get device surface support for device {} (named {}) and queue {}", i,
                              m_devices[i].DeviceProperties.deviceName, q);
            }
        }

        // Surface formats
        glm::u32 numSurfaceFormats = 0;
        res = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &numSurfaceFormats, nullptr);
        if (res != VK_SUCCESS)
        {
            spdlog::error(
                "Failed to get number surface formats for device {} (named {})",
                i,
                m_devices[i].DeviceProperties.deviceName
            );
        }
        if (numSurfaceFormats <= 0)
        {
            spdlog::error(""
                          "Device {} (named {}) cannot be used because it supports {} (less than 1) surface formats. Device Info:\n{}",
                          i,
                          m_devices[i].DeviceProperties.deviceName,
                          numSurfaceFormats,
                          logMessage);
            continue;
        }

        m_devices[i].SurfaceFormats.resize(numSurfaceFormats);

        res = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &numSurfaceFormats,
                                                   m_devices[i].SurfaceFormats.data());
        if (res != VK_SUCCESS)
        {
            spdlog::error(
                "Failed to get surface formats for device {} (named {}) ({} found)",
                i,
                m_devices[i].DeviceProperties.deviceName,
                numSurfaceFormats
            );
        }

        for (glm::u32 j = 0; j < numSurfaceFormats; j++)
        {
            const VkSurfaceFormatKHR& surfaceFormat = m_devices[i].SurfaceFormats[j];
            logMessage += std::format(
                "    Surface Format {} color space {}\n",
                static_cast<int>(surfaceFormat.format),
                static_cast<int>(surfaceFormat.colorSpace)
            );
        }

        // Surface capabilities
        res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &(m_devices[i].SurfaceCapabilities));
        if (res != VK_SUCCESS)
        {
            spdlog::error("Failed to get device capabilities");
        }

        // Present modes
        glm::u32 numSurfacePresentModes = 0;

        res = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &numSurfacePresentModes, nullptr);
        if (res != VK_SUCCESS)
        {
            spdlog::error(
                "Failed to get number surface present modes for device {} (named {})",
                i,
                m_devices[i].DeviceProperties.deviceName
            );
        }
        if (numSurfacePresentModes <= 0)
        {
            spdlog::error(
                "Device {} (named {}) cannot be used because it supports {} (less than 1) surface present modes. Device Info:\n{}",
                i,
                m_devices[i].DeviceProperties.deviceName,
                numSurfaceFormats,
                logMessage);
            continue;
        }

        m_devices[i].PresentModes.resize(numSurfacePresentModes);

        res = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &numSurfacePresentModes,
                                                        m_devices[i].PresentModes.data());
        if (res != VK_SUCCESS)
        {
            spdlog::error(
                "Failed to get surface present modes for device {} (named {}), found {}",
                i,
                m_devices[i].DeviceProperties.deviceName,
                numSurfacePresentModes
            );
        }

        logMessage += std::format("    Number of presentation modes {}\n", numSurfacePresentModes);

        // Memory properties
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &(m_devices[i].MemoryProperties));

        logMessage += std::format("    Num memory types {}\n\n", m_devices[i].MemoryProperties.memoryTypeCount);
        for (glm::u32 j = 0; j < m_devices[i].MemoryProperties.memoryTypeCount; j++)
        {
            logMessage += std::format(
                "    {}: flags {} heap {}\n",
                j,
                m_devices[i].MemoryProperties.memoryTypes[j].propertyFlags,
                m_devices[i].MemoryProperties.memoryTypes[j].heapIndex);
        }
        logMessage += std::format("\n    Num heap types {}\n", m_devices[i].MemoryProperties.memoryHeapCount);

        m_devices[i].DepthFormat = FindDepthFormat(m_devices[i].PhysicalDeviceHandle);
        logMessage += std::format("\n    Depth Format {}", static_cast<int>(m_devices[i].DepthFormat));

        if (i + 1 < numDevices) logMessage += "\n";
        if (logDeviceInfo) spdlog::info(logMessage);

        vkGetPhysicalDeviceFeatures(m_devices[i].PhysicalDeviceHandle, &m_devices[i].Features);
    }

    m_initialized = true;
    spdlog::info("Initialized RenderingDeviceManager!");
}

RenderingDeviceManager::~RenderingDeviceManager()
{
    spdlog::info("Destroyed RenderingDeviceManager");
}

// ReSharper disable once CppDFAConstantFunctionResult
bool RenderingDeviceManager::IsValid() const
{
    return m_initialized;
}

glm::u32 RenderingDeviceManager::SelectDevice(VkQueueFlags requiredQueueType, bool supportsPresent)
{
    for (glm::u32 i = 0; i < m_devices.size(); i++)
    {
        for (glm::u32 queueFamilyIndex = 0; queueFamilyIndex < m_devices[i].QueueFamilyProperties.size();
             queueFamilyIndex++)
        {
            const VkQueueFamilyProperties& queueFamilyProperties = m_devices[i].QueueFamilyProperties[queueFamilyIndex];

            bool isRequiredQueueType = queueFamilyProperties.queueFlags & requiredQueueType;
            bool isRequestedPresentSupport = static_cast<bool>(m_devices[i].QueueSupportsPresent[queueFamilyIndex]) ==
                supportsPresent;
            if (isRequiredQueueType && isRequestedPresentSupport)
            {
                m_deviceIndex = i;
                spdlog::info(
                    "Selected GFX device {} ({}) and queue family {}",
                    m_deviceIndex,
                    m_devices[i].DeviceProperties.deviceName,
                    queueFamilyIndex
                );
                return queueFamilyIndex;
            }
        }
    }

    spdlog::info("Required queue type {} and supports present {} not found\n", requiredQueueType, supportsPresent);

    return 0;
}

const PhysicalDevice& RenderingDeviceManager::Selected() const
{
    DEBUG_ASSERT(m_initialized);
    DEBUG_ASSERT(m_deviceIndex>=0 && "No device selected!");
    DEBUG_ASSERT(m_deviceIndex<m_devices.size() && "Device index out of bounds!");

    return m_devices[m_deviceIndex];
}

bool RenderingDeviceManager::IsExtensionSupported(const PhysicalDevice& device, const char* extensionName) const
{
    for (VkExtensionProperties extensionProperties : device.Extensions)
    {
        if (std::string(extensionProperties.extensionName) == extensionName) return true;
    }
    return false;
}

VkFormat RenderingDeviceManager::FindDepthFormat(VkPhysicalDevice device) const
{
    std::vector candidates = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT
    };

    VkFormat depthFormat = FindSupportedFormat(device, candidates, VK_IMAGE_TILING_OPTIMAL,
                                               VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

    return depthFormat;
}

VkFormat RenderingDeviceManager::FindSupportedFormat(VkPhysicalDevice device, const std::vector<VkFormat>& candidates,
                                                     VkImageTiling tiling, VkFormatFeatureFlags requestedFeatures) const
{
    VkFormat candidateFormat = VK_FORMAT_UNDEFINED;
    for (int i = 0; i < candidates.size(); i++)
    {
        candidateFormat = candidates[i];
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(device, candidateFormat, &properties);

        if ((tiling == VK_IMAGE_TILING_LINEAR) &&
            (properties.linearTilingFeatures & requestedFeatures) == requestedFeatures)
        {
            return candidateFormat;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
            (properties.optimalTilingFeatures & requestedFeatures) == requestedFeatures)
        {
            return candidateFormat;
        }
    }

    spdlog::error("Failed to find supported format!");
    return candidateFormat;
}

std::vector<VkExtensionProperties> RenderingDeviceManager::GetExtensions(VkPhysicalDevice device) const
{
    std::vector<VkExtensionProperties> out;

    glm::u32 numExtensions;
    VkResult res = vkEnumerateDeviceExtensionProperties(device, nullptr, &numExtensions, nullptr);
    if (res != VK_SUCCESS)
    {
        spdlog::error("Failed to get extension count for device");
        return out;
    }
    out.resize(numExtensions);

    res = vkEnumerateDeviceExtensionProperties(device, nullptr, &numExtensions, out.data());
    if (res != VK_SUCCESS)
    {
        spdlog::error("Failed to get extensions (count: {}) for device", numExtensions);
    }
    return out;
}
