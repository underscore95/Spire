#pragma once

#include <libassert/assert.hpp>
#include <vulkan/vulkan.h>
#include <spdlog/spdlog.h>

class VulkanUtils {
public:
    VulkanUtils() = delete;

    static const char *GetDebugSeverityStr(VkDebugUtilsMessageSeverityFlagBitsEXT Severity) {
        switch (Severity) {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
                return "Verbose";

            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                return "Info";

            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                return "Warning";

            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                return "Error";

            default:
                UNREACHABLE();
        }
    }


    static const char *GetDebugType(VkDebugUtilsMessageTypeFlagsEXT Type) {
        switch (Type) {
            case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
                return "General";

            case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
                return "Validation";

            case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
                return "Performance";

#ifdef _WIN64 // doesn't work on my Linux for some reason
            case VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT:
                return "Device address binding";
#endif

            default:
                UNREACHABLE();
        }
    }
};
