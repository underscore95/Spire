#include "VulkanDebugCallback.h"

#include <glm/fwd.hpp>
#include <spdlog/spdlog.h>

#include "VulkanUtils.h"

namespace Spire
{
    VulkanDebugCallback::VulkanDebugCallback(VkInstance instance)
        : m_instance(instance)
    {
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

        if (!vkCreateDebugUtilsMessenger)
        {
            spdlog::error("Cannot find address of vkCreateDebugUtilsMessenger");
            return;
        }

        // Create it
        VkResult res = vkCreateDebugUtilsMessenger(m_instance, &MessengerCreateInfo, nullptr, &m_debugMessenger);
        if (res != VK_SUCCESS)
        {
            spdlog::error("Failed to create vulkan DebugUtilsMessenger");
            return;
        }

        spdlog::info("Debug callback created");
    }

    VulkanDebugCallback::~VulkanDebugCallback()
    {
        if (m_debugMessenger)
        {
            PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessenger = VK_NULL_HANDLE;
            vkDestroyDebugUtilsMessenger = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(
                m_instance, "vkDestroyDebugUtilsMessengerEXT"));
            if (!vkDestroyDebugUtilsMessenger)
            {
                spdlog::error("Cannot find address of vkDestroyDebugUtilsMessenger");
            }
            else
            {
                vkDestroyDebugUtilsMessenger(m_instance, m_debugMessenger, nullptr);
                spdlog::info("Destroyed Debug Callback");
            }
        }
    }

    // ReSharper disable once CppDFAConstantFunctionResult
    VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback::DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT type,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        [[maybe_unused]] void* pUserData
    )
    {
        // Get log level
        spdlog::level::level_enum logLevel;
        switch (severity)
        {
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
        for (glm::u32 i = 0; i < pCallbackData->objectCount; i++)
        {
            objectStr += fmt::format("{:016x} ", pCallbackData->pObjects[i].objectHandle);
        }

        std::string message = fmt::format(
            "Debug callback: '{}'\n Type: {}\n Objects: {}",
            pCallbackData->pMessage,
            VulkanUtils::GetDebugType(type),
            objectStr
        );

        // Log
        spdlog::log(logLevel, message);

        return VK_FALSE; // The function that caused error should not be aborted
    }
}