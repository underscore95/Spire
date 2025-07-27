#include "RenderingManager.h"
#include <glm/glm.hpp>
#include <vector>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

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

    spdlog::info("Initialized RenderingManager!");
}

RenderingManager::~RenderingManager() {
    spdlog::info("Destroying RenderingManager...");

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
           m_device;
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
    spdlog::log(logLevel,  message);

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
