#pragma once

#include "pch.h"

namespace Spire
{
    struct LoadedImage;
    class RenderingManager;
    struct VulkanImage;

    class ImageManager
    {
    public:
        explicit ImageManager(RenderingManager& renderingManager);
        ~ImageManager();

    public:
        VulkanImage CreateImageFromFile(const char* filename);
        void DestroyImage(const VulkanImage& image);

        void CreateImage(VulkanImage& image, glm::uvec2 dimensions, VkImageUsageFlags usage,
                         VkImageUsageFlags propertyFlags, VkFormat format);

        void TransitionImageLayout(const VkImage& image, VkFormat format, VkImageLayout oldLayout,
                                   VkImageLayout newLayout) const;

        VkImageView CreateImageView(VkImage image, VkFormat format, VkFlags aspectFlags) const;

    private:
        [[nodiscard]] glm::u32 GetBytesPerTexFormat(VkFormat format) const;

        void CopyBufferToImage(VkImage dest, VkBuffer source, const glm::uvec2& imageDimensions) const;
        void UpdateImageImage(const VulkanImage& image, const LoadedImage& loadedImage, VkFormat format) const;
        void CreateImageFromData(VulkanImage& image, const LoadedImage& loadedImage, VkFormat format);

        [[nodiscard]] VkSampler CreateImageSampler(VkFilter minFilter, VkFilter maxFilter, VkSamplerAddressMode addressMode) const;

    private:
        RenderingManager& m_renderingManager;
        VkCommandBuffer m_commandBuffer;
        glm::u32 m_numAllocatedImages = 0;
    };
}