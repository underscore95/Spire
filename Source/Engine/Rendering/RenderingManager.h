#pragma once

#include <string>
#include <vulkan/vulkan.h>

class RenderingManager {
public:
    explicit RenderingManager(const std::string &applicationName);

    ~RenderingManager();

public:
    bool IsValid() const;

private:
    void CreateInstance(const std::string &applicationName);

    void CreateDebugCallback();

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT Severity,
        VkDebugUtilsMessageTypeFlagsEXT Type,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData);

private:
    VkInstance m_instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
};
