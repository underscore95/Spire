#pragma once

#include <string>
#include <vulkan/vulkan.h>
#include <memory>

class Window;
class RenderingDeviceManager;

class RenderingManager {
public:
    explicit RenderingManager(const std::string &applicationName, const Window &window);

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

    void DestroyDebugCallback() const;

    void CreateSurface(const Window &window);

private:
    VkInstance m_instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    std::unique_ptr<RenderingDeviceManager> m_deviceManager = nullptr;
};
