#include "RenderingManager.h"
#include <glm/glm.hpp>
#include <vector>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include "RenderingCommandManager.h"
#include "RenderingDeviceManager.h"
#include "VulkanQueue.h"
#include "Engine/Core/Engine.h"
#include "VulkanUtils.h"
#include "Engine/Window/Window.h"
#include "BufferManager.h"
#include "LogicalDevice.h"
#include "Swapchain.h"
#include "TextureManager.h"
#include "VulkanDebugCallback.h"

RenderingManager::RenderingManager(const std::string& applicationName, const Window& window)
{
    spdlog::info("Initializing RenderingManager...");

    GetInstanceVersion();
    CreateInstance(applicationName);
#ifndef NDEBUG
    m_debugCallback = std::make_unique<VulkanDebugCallback>(m_instance);
#endif
    CreateSurface(window);

    m_deviceManager = std::make_unique<RenderingDeviceManager>(m_instance, m_surface, false);

    m_logicalDevice = std::make_unique<LogicalDevice>(*m_deviceManager, m_instanceVersion);

    m_swapchain = std::make_unique<Swapchain>(m_logicalDevice->GetDevice(), m_deviceManager->Selected(),
                                              m_logicalDevice->GetDeviceQueueFamily(), m_surface);

    m_commandManager = std::make_unique<RenderingCommandManager>(m_logicalDevice->GetDeviceQueueFamily(),
                                                                 m_logicalDevice->GetDevice());

    m_queue = std::make_unique<VulkanQueue>(*this, m_logicalDevice->GetDevice(), m_swapchain->GetSwapchain(),
                                            m_logicalDevice->GetDeviceQueueFamily(), 0);

    m_bufferManager = std::make_unique<BufferManager>(*this);
    m_textureManager = std::make_unique<TextureManager>(*this);

    CreateDepthResources(window.GetDimensions());

    spdlog::info("Initialized RenderingManager!");
}

RenderingManager::~RenderingManager()
{
    spdlog::info("Destroying RenderingManager...");

    for (const auto& depthTexture : m_depthImages)
    {
        m_textureManager->DestroyTexture(depthTexture);
    }

    m_textureManager.reset();
    m_bufferManager.reset();

    m_queue.reset();

    m_commandManager.reset();

    m_swapchain.reset();

    m_logicalDevice.reset();
    m_deviceManager.reset();

    if (m_surface)
    {
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        spdlog::info("Destroyed Vulkan surface instance");
    }

    m_debugCallback.reset();

    if (m_instance)
    {
        vkDestroyInstance(m_instance, nullptr);
        spdlog::info("Destroyed Vulkan instance");
    }

    spdlog::info("Destroyed RenderingManager!");
}

bool RenderingManager::IsValid() const
{
    return m_instance &&
        m_surface &&
        m_deviceManager &&
        m_logicalDevice->IsValid() &&
        m_swapchain->IsValid();
}

RenderingCommandManager& RenderingManager::GetCommandManager() const
{
    return *m_commandManager;
}

VkSemaphore RenderingManager::CreateSemaphore() const
{
    VkSemaphoreCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0
    };

    VkSemaphore semaphore;
    VkResult res = vkCreateSemaphore(m_logicalDevice->GetDevice(), &createInfo, nullptr, &semaphore);
    if (res != VK_SUCCESS)
    {
        spdlog::error("Failed to create semaphore");
    }
    DEBUG_ASSERT(semaphore != VK_NULL_HANDLE);
    return semaphore;
}

void RenderingManager::DestroySemaphore(VkSemaphore semaphore) const
{
    vkDestroySemaphore(m_logicalDevice->GetDevice(), semaphore, nullptr);
}

VulkanQueue& RenderingManager::GetQueue() const
{
    return *m_queue;
}

glm::u32 RenderingManager::GetQueueFamily() const
{
    return m_logicalDevice->GetDeviceQueueFamily();
}

VkDevice RenderingManager::GetDevice() const
{
    return m_logicalDevice->GetDevice();
}

const PhysicalDevice& RenderingManager::GetPhysicalDevice() const
{
    return m_deviceManager->Selected();
}

const BufferManager& RenderingManager::GetBufferManager() const
{
    return *m_bufferManager;
}

const TextureManager& RenderingManager::GetTextureManager() const
{
    return *m_textureManager;
}

void RenderingManager::ImageMemoryBarrier(VkCommandBuffer commandBuffer, VkImage image, VkFormat format,
                                          VkImageLayout oldLayout, VkImageLayout newLayout) const
{
    // Copied from the "3D Graphics Rendering Cookbook"
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

const VulkanImage& RenderingManager::GetDepthImage(glm::u32 imageIndex) const
{
    ASSERT(imageIndex < m_depthImages.size());
    return m_depthImages[imageIndex];
}

void RenderingManager::GetInstanceVersion()
{
    VkResult res = vkEnumerateInstanceVersion(&m_instanceVersion.RawVersion);
    if (res != VK_SUCCESS)
    {
        spdlog::error("Failed to get vulkan instance version");
    }

    m_instanceVersion.Variant = VK_API_VERSION_VARIANT(m_instanceVersion.RawVersion);
    m_instanceVersion.Major = VK_API_VERSION_MAJOR(m_instanceVersion.RawVersion);
    m_instanceVersion.Minor = VK_API_VERSION_MINOR(m_instanceVersion.RawVersion);
    m_instanceVersion.Patch = VK_API_VERSION_PATCH(m_instanceVersion.RawVersion);

    spdlog::info("Vulkan loader supports version {}.{}.{}.{}", m_instanceVersion.Variant, m_instanceVersion.Major,
                 m_instanceVersion.Minor, m_instanceVersion.Patch);
}

void RenderingManager::CreateDepthResources(glm::uvec2 windowDimensions)
{
    m_depthImages.resize(m_swapchain->GetNumImages());

    VkFormat depthFormat = m_deviceManager->Selected().DepthFormat;

    for (int i = 0; i < m_depthImages.size(); i++)
    {
        VkImageUsageFlagBits usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        VkMemoryPropertyFlagBits propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        m_textureManager->CreateImage(m_depthImages[i], windowDimensions, usage, propertyFlags, depthFormat);

        m_textureManager->TransitionImageLayout(m_depthImages[i].Image, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED,
                                                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

        m_depthImages[i].ImageView = m_textureManager->CreateImageView(m_depthImages[i].Image, depthFormat,
                                                                       VK_IMAGE_ASPECT_DEPTH_BIT);
    }

    spdlog::info("Created {} depth images", m_depthImages.size());
}

// https://github.com/emeiri/ogldev/blob/VULKAN_02/Vulkan/VulkanCore/Source/core.cpp
void RenderingManager::CreateInstance(const std::string& applicationName)
{
    std::vector Layers = {
        "VK_LAYER_KHRONOS_validation"
    };

    std::vector Extensions = {
        VK_KHR_SURFACE_EXTENSION_NAME,
#if defined (_WIN32)
        "VK_KHR_win32_surface",
#endif
#if defined (__APPLE__)
        "VK_MVK_macos_surface",
#endif
#if defined (__linux__)
        "VK_KHR_xcb_surface",
#endif
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
    };

    ASSERT(
        m_instanceVersion.RawVersion == VK_MAKE_API_VERSION(m_instanceVersion.Variant, m_instanceVersion.Major,
            m_instanceVersion.Minor, m_instanceVersion.Patch));

    VkApplicationInfo AppInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = applicationName.c_str(),
        .applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
        .pEngineName = Engine::s_engineName.c_str(),
        .engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
        .apiVersion = m_instanceVersion.RawVersion
    };

    VkInstanceCreateInfo CreateInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0, // reserved for future use. Must be zero
        .pApplicationInfo = &AppInfo,
        .enabledLayerCount = static_cast<glm::u32>(Layers.size()),
        .ppEnabledLayerNames = Layers.data(),
        .enabledExtensionCount = static_cast<glm::u32>(Extensions.size()),
        .ppEnabledExtensionNames = Extensions.data()
    };

    VkResult res = vkCreateInstance(&CreateInfo, nullptr, &m_instance);
    if (res != VK_SUCCESS)
    {
        spdlog::error("Failed to create Vulkan instance");
        return;
    }

    spdlog::info("Created Vulkan instance");
}

void RenderingManager::CreateSurface(const Window& window)
{
    VkResult result = glfwCreateWindowSurface(m_instance, window.GLFWWindow(), nullptr, &m_surface);
    if (result != VK_SUCCESS)
    {
        spdlog::error("Failed to create Vulkan window surface");
    }
    else
    {
        spdlog::info("Created Vulkan window surface");
    }
}

Swapchain& RenderingManager::GetSwapchain() const
{
    return *m_swapchain;
}
