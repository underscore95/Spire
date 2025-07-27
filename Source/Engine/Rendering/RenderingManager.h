#pragma once

#include <string>
#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include <glm/fwd.hpp>

class Window;
class RenderingDeviceManager;

class RenderingManager {
public:
    explicit RenderingManager(const std::string &applicationName, const Window &window);

    ~RenderingManager();

public:
    [[nodiscard]] bool IsValid() const;

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

    void CreateLogicalDevice();

    void CreateSwapChain();

private:
    VkInstance m_instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    std::unique_ptr<RenderingDeviceManager> m_deviceManager = nullptr;
    const glm::u32 INVALID_DEVICE_QUEUE_FAMILY = -1; // underflow
    glm::u32 m_deviceQueueFamily = INVALID_DEVICE_QUEUE_FAMILY;
    VkDevice m_device = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
    std::vector<VkImage> m_images;
    std::vector<VkImageView> m_imageViews;
};
