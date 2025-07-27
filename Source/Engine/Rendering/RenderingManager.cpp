#include "RenderingManager.h"
#include <glm/glm.hpp>
#include <vector>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include "RenderingCommandManager.h"
#include "RenderingDeviceManager.h"
#include "Engine/Core/Engine.h"
#include "VulkanUtils.h"
#include "Engine/Window/Window.h"

RenderingManager::RenderingManager(const std::string &applicationName, const Window &window) {
    spdlog::info("Initializing RenderingManager...");

    CreateInstance(applicationName);
    CreateDebugCallback();
    CreateSurface(window);

    m_deviceManager = std::make_unique<RenderingDeviceManager>(m_instance, m_surface, false);
    m_deviceQueueFamily = m_deviceManager->SelectDevice(VK_QUEUE_GRAPHICS_BIT, true);
    CreateLogicalDevice();

    CreateSwapChain();

    m_commandManager = std::make_unique<RenderingCommandManager>(m_deviceQueueFamily, m_device);

    spdlog::info("Initialized RenderingManager!");
}

RenderingManager::~RenderingManager() {
    spdlog::info("Destroying RenderingManager...");

    m_commandManager.reset();

    for (auto &m_imageView: m_imageViews) {
        vkDestroyImageView(m_device, m_imageView, nullptr);
    }
    // ReSharper disable once CppDFAConstantConditions
    if (m_swapChain) {
        // ReSharper disable once CppDFAUnreachableCode
        vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
        spdlog::info("Destroyed swapchain");
    }

    if (m_device) {
        vkDestroyDevice(m_device, nullptr);
        spdlog::info("Destroyed logical device");
    }
    m_deviceManager.reset();

    if (m_surface) {
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        spdlog::info("Destroyed Vulkan surface instance");
    }

    DestroyDebugCallback();

    if (m_instance) {
        vkDestroyInstance(m_instance, nullptr);
        spdlog::info("Destroyed Vulkan instance");
    }

    spdlog::info("Destroyed RenderingManager!");
}

bool RenderingManager::IsValid() const {
    return m_instance &&
           m_debugMessenger &&
           m_surface &&
           m_deviceManager &&
           m_deviceQueueFamily != INVALID_DEVICE_QUEUE_FAMILY &&
           m_device &&
           // ReSharper disable once CppDFAConstantConditions
           m_swapChain &&
           // ReSharper disable once CppDFAUnreachableCode
           !m_images.empty() &&
           // ReSharper disable once CppDFAUnreachableCode
           !m_imageViews.empty();
}

glm::u32 RenderingManager::GetNumImages() const {
    return m_images.size();
}

const VkImage &RenderingManager::GetImage(glm::u32 index) const {
    ASSERT(index<m_images.size());
    return m_images[index];
}

RenderingCommandManager &RenderingManager::GetCommandManager() const {
    return *m_commandManager;
}


// https://github.com/emeiri/ogldev/blob/VULKAN_02/Vulkan/VulkanCore/Source/core.cpp
void RenderingManager::CreateInstance(const std::string &applicationName) {
    std::vector Layers = {
        "VK_LAYER_KHRONOS_validation"
    };

    std::vector Extensions = {
        VK_KHR_SURFACE_EXTENSION_NAME,
#if defined (_WIN32)
        "VK_KHR_win32_surface",
#endif
#if defined (__APPLE__)
        "VK_MVK_macos_surface",
#endif
#if defined (__linux__)
        "VK_KHR_xcb_surface",
#endif
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
    };

    VkApplicationInfo AppInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = applicationName.c_str(),
        .applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
        .pEngineName = Engine::s_engineName.c_str(),
        .engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
        .apiVersion = VK_API_VERSION_1_0
    };

    VkInstanceCreateInfo CreateInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0, // reserved for future use. Must be zero
        .pApplicationInfo = &AppInfo,
        .enabledLayerCount = static_cast<glm::u32>(Layers.size()),
        .ppEnabledLayerNames = Layers.data(),
        .enabledExtensionCount = static_cast<glm::u32>(Extensions.size()),
        .ppEnabledExtensionNames = Extensions.data()
    };

    VkResult res = vkCreateInstance(&CreateInfo, nullptr, &m_instance);
    if (res != VK_SUCCESS) {
        spdlog::error("Failed to create Vulkan instance");
        return;
    }

    spdlog::info("Created Vulkan instance");
}

// ReSharper disable once CppDFAConstantFunctionResult
VKAPI_ATTR VkBool32 VKAPI_CALL RenderingManager::DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT Severity,
    VkDebugUtilsMessageTypeFlagsEXT Type,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData
) {
    // Get log level
    spdlog::level::level_enum logLevel;
    switch (Severity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: logLevel = spdlog::level::debug;
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: logLevel = spdlog::level::info;
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: logLevel = spdlog::level::warn;
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: logLevel = spdlog::level::err;
            break;
        default: logLevel = spdlog::level::info;
            break;
    }

    if (logLevel == spdlog::level::info) return VK_FALSE;

    // Build string
    std::string objectStr;
    for (glm::u32 i = 0; i < pCallbackData->objectCount; i++) {
        objectStr += fmt::format("{:016x} ", pCallbackData->pObjects[i].objectHandle);
    }

    std::string message = fmt::format(
        "Debug callback: '{}'\n Type: {}\n Objects: {}",
        pCallbackData->pMessage,
        VulkanUtils::GetDebugType(Type),
        objectStr
    );

    // Log
    spdlog::log(logLevel, message);

    return VK_FALSE; // The function that caused error should not be aborted
}

void RenderingManager::DestroyDebugCallback() const {
    if (m_debugMessenger) {
        PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessenger = VK_NULL_HANDLE;
        vkDestroyDebugUtilsMessenger = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(
            m_instance, "vkDestroyDebugUtilsMessengerEXT"));
        if (!vkDestroyDebugUtilsMessenger) {
            spdlog::error("Cannot find address of vkDestroyDebugUtilsMessenger");
        } else {
            vkDestroyDebugUtilsMessenger(m_instance, m_debugMessenger, nullptr);
            spdlog::info("Destroyed Debug Callback");
        }
    }
}

void RenderingManager::CreateSurface(const Window &window) {
    VkResult result = glfwCreateWindowSurface(m_instance, window.GLFWWindow(), nullptr, &m_surface);
    if (result != VK_SUCCESS) {
        spdlog::error("Failed to create Vulkan window surface");
    } else {
        spdlog::info("Created Vulkan window surface");
    }
}

void RenderingManager::CreateLogicalDevice() {
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

    if (m_deviceManager->Selected().Features.geometryShader == VK_FALSE) {
        spdlog::error("The Geometry Shader is not supported!");
        return;
    }

    if (m_deviceManager->Selected().Features.tessellationShader == VK_FALSE) {
        spdlog::error("The Tessellation Shader is not supported!");
        return;
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.geometryShader = VK_TRUE;
    deviceFeatures.tessellationShader = VK_TRUE;

    VkDeviceCreateInfo deviceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = nullptr,
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
        m_deviceManager->Selected().PhysicalDeviceHandle,
        &deviceCreateInfo, nullptr,
        &m_device
    );
    if (result != VK_SUCCESS) {
        spdlog::error("Failed to create logical device");
    } else {
        spdlog::info("Created logical device");
    }
}

static VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR> &PresentModes) {
    for (const auto &PresentMode: PresentModes) {
        if (PresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return PresentMode;
        }
    }

    // Fallback to FIFO which is always supported
    return VK_PRESENT_MODE_FIFO_KHR;
}


static glm::u32 ChooseNumImages(const VkSurfaceCapabilitiesKHR &Capabilities) {
    glm::u32 requestedNumImages = Capabilities.minImageCount + 1;

    glm::u32 finalNumImages = 0;

    if ((Capabilities.maxImageCount > 0) && (requestedNumImages > Capabilities.maxImageCount)) {
        finalNumImages = Capabilities.maxImageCount;
    } else {
        finalNumImages = requestedNumImages;
    }

    return finalNumImages;
}


static VkSurfaceFormatKHR ChooseSurfaceFormatAndColorSpace(const std::vector<VkSurfaceFormatKHR> &SurfaceFormats) {
    for (const auto &SurfaceFormat: SurfaceFormats) {
        if ((SurfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB) &&
            (SurfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)) {
            return SurfaceFormat;
        }
    }

    return SurfaceFormats[0];
}

VkImageView CreateImageView(VkDevice Device, VkImage Image, VkFormat Format, VkImageAspectFlags AspectFlags,
                            VkImageViewType ViewType, glm::u32 LayerCount, glm::u32 mipLevels) {
    VkImageViewCreateInfo ViewInfo =
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .image = Image,
        .viewType = ViewType,
        .format = Format,
        .components = {
            .r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .a = VK_COMPONENT_SWIZZLE_IDENTITY
        },
        .subresourceRange = {
            .aspectMask = AspectFlags,
            .baseMipLevel = 0,
            .levelCount = mipLevels,
            .baseArrayLayer = 0,
            .layerCount = LayerCount
        }
    };

    VkImageView ImageView;
    VkResult res = vkCreateImageView(Device, &ViewInfo, nullptr, &ImageView);
    if (res != VK_SUCCESS) {
        spdlog::error("Failed to create image view");
    }
    return ImageView;
}

void RenderingManager::CreateSwapChain() {
    // Create swapchain
    const VkSurfaceCapabilitiesKHR &surfaceCapabilities = m_deviceManager->Selected().SurfaceCapabilities;

    glm::u32 numImages = ChooseNumImages(surfaceCapabilities);

    const std::vector<VkPresentModeKHR> &PresentModes = m_deviceManager->Selected().PresentModes;
    VkPresentModeKHR presentMode = ChoosePresentMode(PresentModes);

    VkSurfaceFormatKHR surfaceFormat = ChooseSurfaceFormatAndColorSpace(m_deviceManager->Selected().SurfaceFormats);

    VkSwapchainCreateInfoKHR SwapChainCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .surface = m_surface,
        .minImageCount = numImages,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = surfaceCapabilities.currentExtent,
        .imageArrayLayers = 1,
        .imageUsage = (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT),
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 1,
        .pQueueFamilyIndices = &m_deviceQueueFamily,
        .preTransform = surfaceCapabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode,
        .clipped = VK_TRUE
    };

    VkResult res = vkCreateSwapchainKHR(m_device, &SwapChainCreateInfo, nullptr, &m_swapChain);
    if (res != VK_SUCCESS) {
        spdlog::error("Failed to create Vulkan swapchain");
    } else {
        spdlog::info("Created Vulkan swapchain");
        ASSERT(m_swapChain);
    }

    // Create swapchain images
    glm::u32 numSwapChainImages = 0;
    res = vkGetSwapchainImagesKHR(m_device, m_swapChain, &numSwapChainImages, nullptr);
    if (res != VK_SUCCESS) {
        spdlog::error("Failed to get number of swapchain images");
    }
    ASSERT(numImages == numSwapChainImages);

    spdlog::info("Number of swapchain images {}", numSwapChainImages);

    m_images.resize(numSwapChainImages);
    m_imageViews.resize(numSwapChainImages);

    res = vkGetSwapchainImagesKHR(m_device, m_swapChain, &numSwapChainImages, m_images.data());
    if (res != VK_SUCCESS) {
        spdlog::error("Failed to get swapchain images ({} known)", numSwapChainImages);
    }

    for (glm::u32 i = 0; i < numSwapChainImages; i++) {
        int mipLevels = 1;
        int layerCount = 1;
        m_imageViews[i] = CreateImageView(
            m_device,
            m_images[i],
            surfaceFormat.format,
            VK_IMAGE_ASPECT_COLOR_BIT,
            VK_IMAGE_VIEW_TYPE_2D,
            layerCount,
            mipLevels
        );
    }
}

void RenderingManager::CreateDebugCallback() {
    // Debug message create info
    VkDebugUtilsMessengerCreateInfoEXT MessengerCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .pNext = nullptr,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = &DebugCallback,
        .pUserData = nullptr
    };

    // Get address of create debug utils messenger
    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessenger = VK_NULL_HANDLE;

    vkCreateDebugUtilsMessenger = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(
            m_instance,
            "vkCreateDebugUtilsMessengerEXT"
        )
    );

    if (!vkCreateDebugUtilsMessenger) {
        spdlog::error("Cannot find address of vkCreateDebugUtilsMessenger");
        return;
    }

    // Create it
    VkResult res = vkCreateDebugUtilsMessenger(m_instance, &MessengerCreateInfo, nullptr, &m_debugMessenger);
    if (res != VK_SUCCESS) {
        spdlog::error("Failed to create vulkan DebugUtilsMessenger");
        return;
    }

    spdlog::info("Debug callback created");
}
