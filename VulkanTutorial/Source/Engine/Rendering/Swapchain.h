#pragma once

#include <vector>
#include <glm/fwd.hpp>
#include <vulkan/vulkan_core.h>

class Window;
struct PhysicalDevice;

class Swapchain
{
public:
    Swapchain(VkDevice device,
              const PhysicalDevice& physicalDevice,
              glm::u32 deviceQueueFamily,
              VkSurfaceKHR surface,
              const Window& window);
    ~Swapchain();

public:
    [[nodiscard]] glm::u32 GetNumImages() const;

    [[nodiscard]] const VkImage& GetImage(glm::u32 index) const;

    [[nodiscard]] VkImageView GetImageView(glm::u32 index) const;

    [[nodiscard]] VkSurfaceFormatKHR GetSurfaceFormat() const;

    [[nodiscard]] VkSwapchainKHR GetSwapchain() const;

    [[nodiscard]] bool IsValid() const;

private:
    VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& presentModes) const;
    glm::u32 ChooseNumImages(const VkSurfaceCapabilitiesKHR& capabilities) const;
    VkSurfaceFormatKHR ChooseSurfaceFormatAndColorSpace(const std::vector<VkSurfaceFormatKHR>& surfaceFormats) const;
    VkImageView CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags,
                                VkImageViewType viewType, glm::u32 layerCount, glm::u32 mipLevels) const;

private:
    VkDevice m_device;

    VkSurfaceFormatKHR m_swapChainSurfaceFormat = {};
    VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
    std::vector<VkImage> m_images;
    std::vector<VkImageView> m_imageViews;
};
