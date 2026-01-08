#include "VulkanDebugCallback.h"

#include "Utils/Log.h"

#include "VulkanUtils.h"

namespace Spire {
    VulkanDebugCallback::VulkanDebugCallback(VkInstance instance)
        : m_instance(instance) {
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
            error("Cannot find address of vkCreateDebugUtilsMessenger");
            return;
        }

        // Create it
        VkResult res = vkCreateDebugUtilsMessenger(m_instance, &MessengerCreateInfo, nullptr, &m_debugMessenger);
        if (res != VK_SUCCESS) {
            error("Failed to create vulkan DebugUtilsMessenger");
            return;
        }

        info("Debug callback created");
    }

    VulkanDebugCallback::~VulkanDebugCallback() {
        if (m_debugMessenger) {
            PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessenger = VK_NULL_HANDLE;
            vkDestroyDebugUtilsMessenger = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(
                m_instance, "vkDestroyDebugUtilsMessengerEXT"));
            if (!vkDestroyDebugUtilsMessenger) {
                error("Cannot find address of vkDestroyDebugUtilsMessenger");
            } else {
                vkDestroyDebugUtilsMessenger(m_instance, m_debugMessenger, nullptr);
                info("Destroyed Debug Callback");
            }
        }
    }

    // ReSharper disable once CppDFAConstantFunctionResult
    VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback::DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT type,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        [[maybe_unused]] void *pUserData
    ) {
#ifndef SPIRE_VULKAN_PERFORMANCE_DEBUG_LOGS
        if (type == VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) return VK_FALSE;
#endif

        // Build string
        std::string objectStr;
        for (glm::u32 i = 0; i < pCallbackData->objectCount; i++) {
            objectStr += std::format("{:016x} ", pCallbackData->pObjects[i].objectHandle);
        }

        std::string message = std::format(
            "Debug callback: '{}'\n Type: {}\n Objects: {}",
            pCallbackData->pMessage,
            VulkanUtils::GetDebugType(type),
            objectStr
        );

        // Log
        switch (severity) {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
                if constexpr (LOG_VERBOSE) {
                    info(message);
                }
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                if constexpr (LOG_INFO) {
                    info(message);
                }
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                warn(message);
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                error(message);
                break;
            default:
                info(message);
                break;
        }

        return VK_FALSE; // The function that caused error should not be aborted
    }
}
