#include "TextureManager.h"
#include <libassert/assert.hpp>
#include <spdlog/spdlog.h>
#include "BufferManager.h"
#include "RenderingCommandManager.h"
#include "RenderingManager.h"
#include "RenderingSync.h"
#include "VulkanAllocator.h"
#include "VulkanBuffer.h"
#include "VulkanImage.h"
#include "VulkanQueue.h"
#include "../Resources/ImageLoader.h"

TextureManager::TextureManager(RenderingManager& renderingManager)
    : m_renderingManager(renderingManager)
{
    m_renderingManager.GetCommandManager().CreateCommandBuffers(1, &m_commandBuffer);
}

TextureManager::~TextureManager()
{
    DEBUG_ASSERT(m_numAllocatedImages == 0);
    ASSERT(&m_renderingManager.GetCommandManager() != nullptr);
    ASSERT(&m_renderingManager.GetBufferManager() != nullptr);
    m_renderingManager.GetCommandManager().FreeCommandBuffers(1, &m_commandBuffer);
}

VulkanImage TextureManager::CreateImageFromFile(const char* filename)
{
    // Load image
    VulkanImage texture = {};
    LoadedImage loadedImage = ImageLoader::LoadImage(filename);
    if (!loadedImage.IsValid())
    {
        return texture;
    }

    // Create vulkan texture
    constexpr VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
    CreateTextureImageFromData(texture, loadedImage, format);

    // Unload image
    ImageLoader::UnloadImage(loadedImage);

    // Create image view
    VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
    texture.ImageView = CreateImageView(texture.Image, format, aspectFlags);

    // Create sampler
    VkFilter minFilter = VK_FILTER_LINEAR;
    VkFilter maxFilter = VK_FILTER_LINEAR;
    VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    texture.Sampler = CreateTextureSampler(minFilter, maxFilter, addressMode);

    spdlog::info("Created vulkan texture from '{}'", filename);
    return texture;
}

void TextureManager::DestroyImage(const VulkanImage& vulkanTexture)
{
    DEBUG_ASSERT(m_numAllocatedImages > 0);
    VkDevice device = m_renderingManager.GetDevice();
    vkDestroySampler(device, vulkanTexture.Sampler, nullptr);
    vkDestroyImageView(device, vulkanTexture.ImageView, nullptr);
    vmaDestroyImage(
        m_renderingManager.GetAllocatorWrapper().GetAllocator(),
        vulkanTexture.Image,
        vulkanTexture.Allocation
    );
    m_numAllocatedImages--;
}

void TextureManager::CreateImage(VulkanImage& texture, glm::uvec2 dimensions,
                                 VkImageUsageFlags usage, VkMemoryPropertyFlags propertyFlags, VkFormat format)
{
    VkImageCreateInfo imageInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent = VkExtent3D{
            .width = dimensions.x,
            .height = dimensions.y,
            .depth = 1
        },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    VmaAllocationCreateInfo allocInfo = {
        .flags = 0,
        .usage = VMA_MEMORY_USAGE_AUTO,
        .requiredFlags = propertyFlags
    };

    VkResult res = vmaCreateImage(
        m_renderingManager.GetAllocatorWrapper().GetAllocator(),
        &imageInfo,
        &allocInfo,
        &texture.Image,
        &texture.Allocation,
        nullptr);

    if (res != VK_SUCCESS)
    {
        spdlog::error("Failed to create vulkan image");
    }else
    {
        m_numAllocatedImages++;
    }
}

glm::u32 TextureManager::GetBytesPerTexFormat(VkFormat format) const
{
    switch (format)
    {
    case VK_FORMAT_R8_SINT:
    case VK_FORMAT_R8_UNORM:
        return 1;
    case VK_FORMAT_R16_SFLOAT:
        return 2;
    case VK_FORMAT_R16G16_SFLOAT:
    case VK_FORMAT_R16G16_SNORM:
    case VK_FORMAT_B8G8R8A8_UNORM:
    case VK_FORMAT_R8G8B8A8_UNORM:
    case VK_FORMAT_R8G8B8A8_SNORM:
    case VK_FORMAT_R8G8B8A8_SRGB:
        return 4;
    case VK_FORMAT_R16G16B16A16_SFLOAT:
        return 4 * sizeof(uint16_t);
    case VK_FORMAT_R32G32B32A32_SFLOAT:
        return 4 * sizeof(float);
    default:
        spdlog::error("Unknown texture format {}", static_cast<int>(format));
        ASSERT(false);
    }

    return 0;
}

void TextureManager::CopyBufferToImage(VkImage dest, VkBuffer source, const glm::uvec2& imageDimensions) const
{
    auto& commandManager = m_renderingManager.GetCommandManager();
    commandManager.BeginCommandBuffer(m_commandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    VkBufferImageCopy bufferImageCopy = {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = VkImageSubresourceLayers{
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1
        },
        .imageOffset = VkOffset3D{.x = 0, .y = 0, .z = 0},
        .imageExtent = VkExtent3D{.width = imageDimensions.x, .height = imageDimensions.y, .depth = 1}
    };

    vkCmdCopyBufferToImage(m_commandBuffer, source, dest, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1, &bufferImageCopy);

    vkEndCommandBuffer(m_commandBuffer);

    m_renderingManager.GetQueue().SubmitSync(m_commandBuffer);

    m_renderingManager.GetQueue().WaitIdle();
}

void TextureManager::UpdateTextureImage(const VulkanImage& texture, const LoadedImage& loadedImage,
                                        VkFormat format) const
{
    glm::u32 bytesPerPixel = GetBytesPerTexFormat(format);

    VkDeviceSize layerSize = loadedImage.Dimensions.x * loadedImage.Dimensions.y * bytesPerPixel;
    constexpr int layerCount = 1;
    VkDeviceSize imageSize = layerCount * layerSize;

    VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VkMemoryPropertyFlags memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    auto& bufferManager = m_renderingManager.GetBufferManager();
    VulkanBuffer stagingBuffer = bufferManager.CreateBuffer(imageSize, usage, memoryProperties);
    bufferManager.UpdateBuffer(stagingBuffer, loadedImage.Data, imageSize);

    TransitionImageLayout(texture.Image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    CopyBufferToImage(texture.Image, stagingBuffer.Buffer, loadedImage.Dimensions);

    TransitionImageLayout(texture.Image, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    bufferManager.DestroyBuffer(stagingBuffer);
}

void TextureManager::CreateTextureImageFromData(VulkanImage& texture, const LoadedImage& loadedImage,
                                                VkFormat format)
{
    constexpr VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    constexpr VkMemoryPropertyFlags propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    CreateImage(texture, loadedImage.Dimensions, usage, propertyFlags, format);

    UpdateTextureImage(texture, loadedImage, format);
}

void TextureManager::TransitionImageLayout(const VkImage& image, VkFormat format, VkImageLayout oldLayout,
                                           VkImageLayout newLayout) const
{
    m_renderingManager.GetCommandManager().BeginCommandBuffer(m_commandBuffer,
                                                              VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    m_renderingManager.GetRenderingSync().ImageMemoryBarrier(m_commandBuffer, image, format, oldLayout, newLayout);

    vkEndCommandBuffer(m_commandBuffer);

    m_renderingManager.GetQueue().SubmitSync(m_commandBuffer);

    m_renderingManager.GetQueue().WaitIdle();
}

VkImageView TextureManager::CreateImageView(VkImage image, VkFormat format, VkFlags aspectFlags) const
{
    VkImageViewCreateInfo viewInfo =
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
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
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    VkImageView imageView;
    VkResult res = vkCreateImageView(m_renderingManager.GetDevice(), &viewInfo, nullptr, &imageView);
    if (res != VK_SUCCESS)
    {
        spdlog::error("Failed to create image view");
    }
    return imageView;
}

VkSampler TextureManager::CreateTextureSampler(VkFilter minFilter, VkFilter maxFilter,
                                               VkSamplerAddressMode addressMode) const
{
    VkSamplerCreateInfo samplerInfo = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .magFilter = minFilter,
        .minFilter = maxFilter,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = addressMode,
        .addressModeV = addressMode,
        .addressModeW = addressMode,
        .mipLodBias = 0.0f,
        .anisotropyEnable = VK_FALSE,
        .maxAnisotropy = 1,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0f,
        .maxLod = 0.0f,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE
    };

    VkSampler sampler;
    VkResult res = vkCreateSampler(m_renderingManager.GetDevice(), &samplerInfo, nullptr, &sampler);
    if (res != VK_SUCCESS)
    {
        spdlog::error("Failed to create texture sampler");
    }
    return sampler;
}
