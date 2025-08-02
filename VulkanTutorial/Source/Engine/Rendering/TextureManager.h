#pragma once

#include <vulkan/vulkan_core.h>
#include <glm/fwd.hpp>

struct LoadedImage;
class RenderingManager;

struct VulkanTexture
{
    VkImage Image = VK_NULL_HANDLE;
    VkDeviceMemory DeviceMemory = VK_NULL_HANDLE;
    VkImageView ImageView = VK_NULL_HANDLE;
    VkSampler Sampler = VK_NULL_HANDLE;
};

class TextureManager
{
public:
    explicit TextureManager(RenderingManager& renderingManager);
    ~TextureManager();

public:
    VulkanTexture CreateTexture(const char* filename) const;
    void DestroyTexture(const VulkanTexture& vulkanTexture) const;

    void CreateImage(VulkanTexture& texture, glm::uvec2 dimensions, VkImageUsageFlags usage,
                     VkImageUsageFlags propertyFlags, VkFormat format) const;

    void TransitionImageLayout(const VkImage& image, VkFormat format, VkImageLayout oldLayout,
                               VkImageLayout newLayout) const;

    VkImageView CreateImageView(VkImage image, VkFormat format, VkFlags aspectFlags) const;

private:
    glm::u32 GetBytesPerTexFormat(VkFormat format) const;

    void CopyBufferToImage(VkImage dest, VkBuffer source, const glm::uvec2& imageDimensions) const;
    void UpdateTextureImage(const VulkanTexture& texture, const LoadedImage& loadedImage, VkFormat format) const;
    void CreateTextureImageFromData(VulkanTexture& texture, const LoadedImage& loadedImage, VkFormat format) const;

    void ImageMemBarrier(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout,
                         VkImageLayout newLayout) const;


    VkSampler CreateTextureSampler(VkFilter minFilter, VkFilter maxFilter, VkSamplerAddressMode addressMode) const;

private:
    RenderingManager& m_renderingManager;
    VkCommandBuffer m_commandBuffer;
};
