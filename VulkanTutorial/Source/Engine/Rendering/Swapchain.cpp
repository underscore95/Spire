// ReSharper doesn't like parsing this file
// ReSharper disable CppDFAUnreachableCode
// ReSharper disable CppDFAConstantFunctionResult
// ReSharper disable CppDFAConstantParameter
// ReSharper disable CppDFAConstantConditions

#include "Swapchain.h"

#include <libassert/assert.hpp>
#include <spdlog/spdlog.h>

#include "RenderingDeviceManager.h"

Swapchain::Swapchain(
    VkDevice device,
    const PhysicalDevice& physicalDevice,
    glm::u32 deviceQueueFamily,
    VkSurfaceKHR surface)
    : m_device(device)
{
    // Create swapchain
    glm::u32 numImages = ChooseNumImages(physicalDevice.SurfaceCapabilities);

    VkPresentModeKHR presentMode = ChoosePresentMode(physicalDevice.PresentModes);

    m_swapChainSurfaceFormat = ChooseSurfaceFormatAndColorSpace(physicalDevice.SurfaceFormats);

    VkSwapchainCreateInfoKHR SwapChainCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .surface = surface,
        .minImageCount = numImages,
        .imageFormat = m_swapChainSurfaceFormat.format,
        .imageColorSpace = m_swapChainSurfaceFormat.colorSpace,
        .imageExtent = physicalDevice.SurfaceCapabilities.currentExtent,
        .imageArrayLayers = 1,
        .imageUsage = (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT),
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 1,
        .pQueueFamilyIndices = &deviceQueueFamily,
        .preTransform = physicalDevice.SurfaceCapabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode,
        .clipped = VK_TRUE
    };

    VkResult res = vkCreateSwapchainKHR(m_device, &SwapChainCreateInfo, nullptr, &m_swapChain);
    if (res != VK_SUCCESS)
    {
        spdlog::error("Failed to create Vulkan swapchain");
    }
    else
    {
        spdlog::info("Created Vulkan swapchain");
        ASSERT(m_swapChain);
    }

    // Create swapchain images
    glm::u32 numSwapChainImages = 0;
    res = vkGetSwapchainImagesKHR(m_device, m_swapChain, &numSwapChainImages, nullptr);
    if (res != VK_SUCCESS)
    {
        spdlog::error("Failed to get number of swapchain images");
    }
    ASSERT(numImages == numSwapChainImages);

    spdlog::info("Number of swapchain images {}", numSwapChainImages);

    m_images.resize(numSwapChainImages);
    m_imageViews.resize(numSwapChainImages);

    res = vkGetSwapchainImagesKHR(m_device, m_swapChain, &numSwapChainImages, m_images.data());
    if (res != VK_SUCCESS)
    {
        spdlog::error("Failed to get swapchain images ({} known)", numSwapChainImages);
    }

    for (glm::u32 i = 0; i < numSwapChainImages; i++)
    {
        int mipLevels = 1;
        int layerCount = 1;
        m_imageViews[i] = CreateImageView(
            m_device,
            m_images[i],
            m_swapChainSurfaceFormat.format,
            VK_IMAGE_ASPECT_COLOR_BIT,
            VK_IMAGE_VIEW_TYPE_2D,
            layerCount,
            mipLevels
        );
    }
}

Swapchain::~Swapchain()
{
    for (auto& m_imageView : m_imageViews)
    {
        vkDestroyImageView(m_device, m_imageView, nullptr);
    }

    if (m_swapChain)
    {
        vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
        spdlog::info("Destroyed swapchain");
    }
}

glm::u32 Swapchain::GetNumImages() const
{
    return m_images.size();
}

const VkImage& Swapchain::GetImage(glm::u32 index) const
{
    ASSERT(index < m_images.size());
    return m_images[index];
}

VkImageView Swapchain::GetImageView(glm::u32 index) const
{
    ASSERT(index < m_imageViews.size());
    return m_imageViews[index];
}

VkSurfaceFormatKHR Swapchain::GetSurfaceFormat() const
{
    return m_swapChainSurfaceFormat;
}

VkSwapchainKHR Swapchain::GetSwapchain() const
{
    return m_swapChain;
}

bool Swapchain::IsValid() const
{
    return m_swapChain && !m_images.empty() && !m_imageViews.empty();
}

VkPresentModeKHR Swapchain::ChoosePresentMode(const std::vector<VkPresentModeKHR>& presentModes) const
{
    for (const auto& presentMode : presentModes)
    {
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return presentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

glm::u32 Swapchain::ChooseNumImages(const VkSurfaceCapabilitiesKHR& capabilities) const
{
    glm::u32 requestedNumImages = capabilities.minImageCount + 1;
    glm::u32 finalNumImages = 0;

    if ((capabilities.maxImageCount > 0) && (requestedNumImages > capabilities.maxImageCount))
    {
        finalNumImages = capabilities.maxImageCount;
    }
    else
    {
        finalNumImages = requestedNumImages;
    }

    return finalNumImages;
}

VkSurfaceFormatKHR Swapchain::ChooseSurfaceFormatAndColorSpace(
    const std::vector<VkSurfaceFormatKHR>& surfaceFormats) const
{
    for (const auto& surfaceFormat : surfaceFormats)
    {
        if ((surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB) &&
            (surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR))
        {
            return surfaceFormat;
        }
    }

    return surfaceFormats[0];
}

VkImageView Swapchain::CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags,
                                       VkImageViewType viewType, glm::u32 layerCount, glm::u32 mipLevels) const

{
    VkImageViewCreateInfo viewInfo =
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .image = image,
        .viewType = viewType,
        .format = format,
        .components = {
            .r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .a = VK_COMPONENT_SWIZZLE_IDENTITY
        },
        .subresourceRange = {
            .aspectMask = aspectFlags,
            .baseMipLevel = 0,
            .levelCount = mipLevels,
            .baseArrayLayer = 0,
            .layerCount = layerCount
        }
    };

    VkImageView imageView;
    VkResult res = vkCreateImageView(device, &viewInfo, nullptr, &imageView);
    if (res != VK_SUCCESS)
    {
        spdlog::error("Failed to create image view");
    }
    return imageView;
}
