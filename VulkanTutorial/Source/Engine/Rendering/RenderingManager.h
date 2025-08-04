#pragma once

#include <string>
#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include <glm/fwd.hpp>

struct VulkanImage;
class TextureManager;
class RenderingCommandManager;
class Window;
class RenderingDeviceManager;
struct PhysicalDevice;
class VulkanQueue;
class BufferManager;

class RenderingManager
{
public:
    explicit RenderingManager(const std::string& applicationName, const Window& window);

    ~RenderingManager();

public:
    [[nodiscard]] bool IsValid() const;

    [[nodiscard]] glm::u32 GetNumImages() const;

    [[nodiscard]] const VkImage& GetImage(glm::u32 index) const;

    [[nodiscard]] VkImageView GetImageView(glm::u32 index) const;

    [[nodiscard]] RenderingCommandManager& GetCommandManager() const;

    [[nodiscard]] VkSemaphore CreateSemaphore() const;

    void DestroySemaphore(VkSemaphore semaphore) const;

    [[nodiscard]] VulkanQueue& GetQueue() const;

    [[nodiscard]] glm::u32 GetQueueFamily() const;

    [[nodiscard]] VkDevice GetDevice() const;

    [[nodiscard]] const PhysicalDevice& GetPhysicalDevice() const;

    [[nodiscard]] const BufferManager& GetBufferManager() const;

    [[nodiscard]] const TextureManager& GetTextureManager() const;

    [[nodiscard]] VkSurfaceFormatKHR GetSwapChainSurfaceFormat() const;

    void ImageMemoryBarrier(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout,
                            VkImageLayout newLayout) const;

    [[nodiscard]] const VulkanImage& GetDepthImage(glm::u32 imageIndex) const;

private:
    void GetInstanceVersion();

    void CreateDepthResources(glm::uvec2 windowDimensions);

    void CreateInstance(const std::string& applicationName);

    void CreateDebugCallback();

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT Severity,
        VkDebugUtilsMessageTypeFlagsEXT Type,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);

    void DestroyDebugCallback() const;

    void CreateSurface(const Window& window);

    void CreateLogicalDevice();

    void CreateSwapChain();

    enum class DynamicRenderingSupport
    {
        NONE, EXTENSION_REQUIRED, SUPPORTED
    };

    [[nodiscard]] DynamicRenderingSupport GetDynamicRenderingSupport() const;

private:
    VkInstance m_instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    std::unique_ptr<RenderingDeviceManager> m_deviceManager = nullptr;
    const glm::u32 INVALID_DEVICE_QUEUE_FAMILY = -1; // underflow
    glm::u32 m_deviceQueueFamily = INVALID_DEVICE_QUEUE_FAMILY;
    VkDevice m_device = VK_NULL_HANDLE;
    VkSurfaceFormatKHR m_swapChainSurfaceFormat = {};
    VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
    std::vector<VkImage> m_images;
    std::vector<VkImageView> m_imageViews;
    std::unique_ptr<RenderingCommandManager> m_commandManager = nullptr;
    std::unique_ptr<VulkanQueue> m_queue = nullptr;
    std::unique_ptr<BufferManager> m_bufferManager = nullptr;
    std::unique_ptr<TextureManager> m_textureManager = nullptr;
    std::vector<VulkanImage> m_depthImages;

    struct
    {
        glm::u32 RawVersion = 0;
        glm::u32 Variant = 0;
        glm::u32 Major = 0;
        glm::u32 Minor = 0;
        glm::u32 Patch = 0;
    } m_instanceVersion;
};
