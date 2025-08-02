#include "TextureManager.h"
#include <libassert/assert.hpp>
#include <spdlog/spdlog.h>
#include "BufferManager.h"
#include "RenderingCommandManager.h"
#include "RenderingManager.h"
#include "VulkanQueue.h"
#include "Engine/Utils/ImageLoader.h"

TextureManager::TextureManager(RenderingManager& renderingManager)
    : m_renderingManager(renderingManager)
{
    m_renderingManager.GetCommandManager().CreateCommandBuffers(1, &m_commandBuffer);
}

TextureManager::~TextureManager()
{
    ASSERT(&m_renderingManager.GetCommandManager() != nullptr);
    ASSERT(&m_renderingManager.GetBufferManager() != nullptr);
    m_renderingManager.GetCommandManager().FreeCommandBuffers(1, &m_commandBuffer);
}

VulkanTexture TextureManager::CreateTexture(const char* filename) const
{
    // Load image
    VulkanTexture texture = {};
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

void TextureManager::DestroyTexture(const VulkanTexture& vulkanTexture) const
{
    VkDevice device = m_renderingManager.GetDevice();
    vkDestroySampler(device, vulkanTexture.Sampler, nullptr);
    vkDestroyImageView(device, vulkanTexture.ImageView, nullptr);
    vkDestroyImage(device, vulkanTexture.Image, nullptr);
    vkFreeMemory(device, vulkanTexture.DeviceMemory, nullptr);
}

void TextureManager::CreateImage(VulkanTexture& texture, glm::uvec2 dimensions,
                                 VkImageUsageFlags usage, VkMemoryPropertyFlags propertyFlags, VkFormat format) const
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

    VkDevice device = m_renderingManager.GetDevice();

    // create the image object
    VkResult res = vkCreateImage(device, &imageInfo, nullptr, &texture.Image);
    if (res != VK_SUCCESS)
    {
        spdlog::error("Failed to create vulkan image");
    }

    // memory info
    VkMemoryRequirements memoryRequirements = {};
    vkGetImageMemoryRequirements(device, texture.Image, &memoryRequirements);

    glm::u32 memoryTypeIndex = m_renderingManager.GetBufferManager().GetMemoryTypeIndex(
        memoryRequirements.memoryTypeBits, propertyFlags);

    // allocate memory
    VkMemoryAllocateInfo memoryAllocationInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = memoryRequirements.size,
        .memoryTypeIndex = memoryTypeIndex
    };

    res = vkAllocateMemory(device, &memoryAllocationInfo, nullptr, &texture.DeviceMemory);
    if (res != VK_SUCCESS)
    {
        spdlog::error("Failed to allocate memory for vulkan image");
    }

    //  bind memory
    res = vkBindImageMemory(device, texture.Image, texture.DeviceMemory, 0);
    if (res != VK_SUCCESS)
    {
        spdlog::error("Failed to bind memory for vulkan image");
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

    m_renderingManager.GetQueue().WaitUntilExecutedAll();
}

void TextureManager::UpdateTextureImage(const VulkanTexture& texture, const LoadedImage& loadedImage,
                                        VkFormat format) const
{
    glm::u32 bytesPerPixel = GetBytesPerTexFormat(format);

    VkDeviceSize layerSize = loadedImage.Dimensions.x * loadedImage.Dimensions.y * bytesPerPixel;
    constexpr int layerCount = 1;
    VkDeviceSize imageSize = layerCount * layerSize;

    VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    auto& bufferManager = m_renderingManager.GetBufferManager();
    VulkanBuffer stagingBuffer = bufferManager.CreateBuffer(imageSize, usage, properties);
    bufferManager.UpdateBuffer(stagingBuffer, loadedImage.Data, imageSize);

    TransitionImageLayout(texture.Image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    CopyBufferToImage(texture.Image, stagingBuffer.Buffer, loadedImage.Dimensions);

    TransitionImageLayout(texture.Image, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    bufferManager.DestroyBuffer(stagingBuffer);
}

void TextureManager::CreateTextureImageFromData(VulkanTexture& texture, const LoadedImage& loadedImage,
                                                VkFormat format) const
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

    ImageMemBarrier(m_commandBuffer, image, format, oldLayout, newLayout);

    vkEndCommandBuffer(m_commandBuffer);

    m_renderingManager.GetQueue().SubmitSync(m_commandBuffer);

    m_renderingManager.GetQueue().WaitUntilExecutedAll();
}

// Copied from the "3D Graphics Rendering Cookbook"
void TextureManager::ImageMemBarrier(VkCommandBuffer commandBuffer, VkImage image, VkFormat format,
                                     VkImageLayout oldLayout, VkImageLayout newLayout) const
{
    const bool hasStencilComponent = ((format == VK_FORMAT_D32_SFLOAT_S8_UINT) || (format ==
        VK_FORMAT_D24_UNORM_S8_UINT));


    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = 0,
        .dstAccessMask = 0,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = VkImageSubresourceRange{
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_NONE;
    VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_NONE;

    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
        (format == VK_FORMAT_D16_UNORM) ||
        (format == VK_FORMAT_X8_D24_UNORM_PACK32) ||
        (format == VK_FORMAT_D32_SFLOAT) ||
        (format == VK_FORMAT_S8_UINT) ||
        (format == VK_FORMAT_D16_UNORM_S8_UINT) ||
        (format == VK_FORMAT_D24_UNORM_S8_UINT))
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (hasStencilComponent)
        {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    else
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_GENERAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } /* Convert back from read-only to updateable */
    else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } /* Convert from updateable texture to shader read-only */
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
        newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } /* Convert depth texture from undefined state to depth-stencil buffer */
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    } /* Wait for render pass to complete */
    else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout ==
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = 0; // VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = 0;
        /*
                sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        ///		destinationStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
                destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        */
        sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } /* Convert back from read-only to color attachment */
    else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout ==
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    } /* Convert from updateable texture to shader read-only */
    else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout ==
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } /* Convert back from read-only to depth attachment */
    else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout ==
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        destinationStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    } /* Convert from updateable depth texture to shader read-only */
    else if (oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL && newLayout ==
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
    {
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = 0;

        sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    }
    else
    {
        spdlog::error("Unknown barrier case\n");
        ASSERT(false);
    }

    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage,
                         0, 0, nullptr, 0, nullptr, 1, &barrier);
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
