#pragma once

#include <vulkan/vulkan.hpp>

namespace Spire {
    class VulkanDebugCallback {
    public:
        explicit VulkanDebugCallback(VkInstance instance);

        ~VulkanDebugCallback();

    private:
        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT severity,
            VkDebugUtilsMessageTypeFlagsEXT type,
            const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
            void *pUserData);

    private:
        static constexpr bool LOG_VERBOSE = false;
        static constexpr bool LOG_INFO = false;

        VkInstance m_instance;

        VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
    };
}
