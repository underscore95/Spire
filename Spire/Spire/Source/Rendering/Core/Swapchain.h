#pragma once

#include "pch.h"

namespace Spire
{
    class Window;
    struct PhysicalDevice;

    class Swapchain
    {
    public:
        Swapchain(VkDevice device,
                  const PhysicalDevice& physicalDevice,
                  glm::u32 deviceQueueFamily,
                  VkSurfaceKHR surface,
                  const Window& window,
                    glm::u32 maximumSwapchainImages);
        ~Swapchain();

    public:
        [[nodiscard]] glm::u32 GetNumImages() const;

        [[nodiscard]] const VkImage& GetImage(glm::u32 index) const;

        [[nodiscard]] VkImageView GetImageView(glm::u32 index) const;

        [[nodiscard]] VkSurfaceFormatKHR GetSurfaceFormat() const;

        [[nodiscard]] VkSwapchainKHR GetSwapchain() const;

        [[nodiscard]] bool IsValid() const;

    private:
        [[nodiscard]] static VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& presentModes);
        static glm::u32 ChooseNumImages(glm::u32 maximumSwapchainImages, const VkSurfaceCapabilitiesKHR &capabilities) ;
        static VkSurfaceFormatKHR ChooseSurfaceFormatAndColorSpace(const std::vector<VkSurfaceFormatKHR>& surfaceFormats) ;

        static VkImageView CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags,
                                           VkImageViewType viewType, glm::u32 layerCount, glm::u32 mipLevels);

    private:
        VkDevice m_device;

        VkSurfaceFormatKHR m_swapChainSurfaceFormat = {};
        VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
        std::vector<VkImage> m_images;
        std::vector<VkImageView> m_imageViews;
    };
}