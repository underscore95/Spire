#pragma once

#include <vulkan/vulkan_core.h>
#include <glm/fwd.hpp>

struct LoadedImage;
class RenderingManager;
struct VulkanImage;

class TextureManager
{
public:
    explicit TextureManager(RenderingManager& renderingManager);
    ~TextureManager();

public:
    VulkanImage CreateImageFromFile(const char* filename);
    void DestroyImage(const VulkanImage& image);

    void CreateImage(VulkanImage& texture, glm::uvec2 dimensions, VkImageUsageFlags usage,
                     VkImageUsageFlags propertyFlags, VkFormat format);

    void TransitionImageLayout(const VkImage& image, VkFormat format, VkImageLayout oldLayout,
                               VkImageLayout newLayout) const;

    VkImageView CreateImageView(VkImage image, VkFormat format, VkFlags aspectFlags) const;

private:
    glm::u32 GetBytesPerTexFormat(VkFormat format) const;

    void CopyBufferToImage(VkImage dest, VkBuffer source, const glm::uvec2& imageDimensions) const;
    void UpdateTextureImage(const VulkanImage& texture, const LoadedImage& loadedImage, VkFormat format) const;
    void CreateTextureImageFromData(VulkanImage& texture, const LoadedImage& loadedImage, VkFormat format);

    VkSampler CreateTextureSampler(VkFilter minFilter, VkFilter maxFilter, VkSamplerAddressMode addressMode) const;

private:
    RenderingManager& m_renderingManager;
    VkCommandBuffer m_commandBuffer;
    glm::u32 m_numAllocatedImages = 0;
};
