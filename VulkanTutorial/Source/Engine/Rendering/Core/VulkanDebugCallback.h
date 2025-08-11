#pragma once

#include <vulkan/vulkan.hpp>

class VulkanDebugCallback
{
public:
    explicit VulkanDebugCallback(VkInstance instance);
    ~VulkanDebugCallback();

private:
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT type,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);

private:
    VkInstance m_instance;

    VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
};
