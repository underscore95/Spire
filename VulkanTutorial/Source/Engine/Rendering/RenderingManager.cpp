// ReSharper disable CppDFATimeOver

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
#include "ImGuiRenderer.h"
#include "LogicalDevice.h"
#include "Renderer.h"
#include "RenderingSync.h"
#include "Swapchain.h"
#include "TextureManager.h"
#include "VulkanAllocator.h"
#include "VulkanDebugCallback.h"
#include "VulkanImage.h"

RenderingManager::RenderingManager(Engine& engine,
                                   const std::string& applicationName,
                                   Window& window)
    : m_engine(engine),
      m_window(window)
{
    spdlog::info("Initializing RenderingManager...");

    m_renderingSync = std::make_unique<RenderingSync>(*this);

    GetInstanceVersion();
    CreateInstance(applicationName);
#ifndef NDEBUG
    m_debugCallback = std::make_unique<VulkanDebugCallback>(m_instance);
#endif
    CreateSurface(window);

    m_deviceManager = std::make_unique<RenderingDeviceManager>(m_instance, m_surface, false);

    m_logicalDevice = std::make_unique<LogicalDevice>(*m_deviceManager, m_instanceVersion);

    m_allocator = std::make_unique<VulkanAllocator>(
        m_logicalDevice->GetDevice(),
        m_deviceManager->Selected().PhysicalDeviceHandle,
        m_instance,
        m_instanceVersion);

    CreateSwapchain();

    m_commandManager = std::make_unique<RenderingCommandManager>(m_logicalDevice->GetDeviceQueueFamily(),
                                                                 m_logicalDevice->GetDevice());

    CreateQueue();

    m_bufferManager = std::make_unique<BufferManager>(*this);
    m_textureManager = std::make_unique<TextureManager>(*this);

    CreateDepthResources();

    m_renderer = std::make_unique<Renderer>(*this, window);
    m_imGuiRenderer = std::make_unique<ImGuiRenderer>(*this, window);

    spdlog::info("Initialized RenderingManager!");
}

RenderingManager::~RenderingManager()
{
    spdlog::info("Destroying RenderingManager...");

    m_imGuiRenderer.reset();
    m_renderer.reset();

    FreeDepthResources();

    m_textureManager.reset();
    m_bufferManager.reset();

    m_queue.reset();

    m_commandManager.reset();

    m_swapchain.reset();

    m_allocator.reset();

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

BufferManager& RenderingManager::GetBufferManager() const
{
    return *m_bufferManager;
}

TextureManager& RenderingManager::GetTextureManager() const
{
    return *m_textureManager;
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

void RenderingManager::CreateDepthResources()
{
    m_depthImages.resize(m_swapchain->GetNumImages());

    VkFormat depthFormat = m_deviceManager->Selected().DepthFormat;

    for (int i = 0; i < m_depthImages.size(); i++)
    {
        VkImageUsageFlagBits usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        VkMemoryPropertyFlagBits propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        m_textureManager->CreateImage(m_depthImages[i], m_window.GetDimensions(), usage, propertyFlags, depthFormat);

        m_textureManager->TransitionImageLayout(m_depthImages[i].Image, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED,
                                                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

        m_depthImages[i].ImageView = m_textureManager->CreateImageView(m_depthImages[i].Image, depthFormat,
                                                                       VK_IMAGE_ASPECT_DEPTH_BIT);
    }

    spdlog::info("Created {} depth images", m_depthImages.size());
}

void RenderingManager::FreeDepthResources()
{
    for (const auto& depthTexture : m_depthImages)
    {
        m_textureManager->DestroyImage(depthTexture);
    }
    m_depthImages.clear();
}

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

    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0, // reserved for future use. Must be zero
        .pApplicationInfo = &AppInfo,
        .enabledLayerCount = static_cast<glm::u32>(Layers.size()),
        .ppEnabledLayerNames = Layers.data(),
        .enabledExtensionCount = static_cast<glm::u32>(Extensions.size()),
        .ppEnabledExtensionNames = Extensions.data()
    };

    VkResult res = vkCreateInstance(&createInfo, nullptr, &m_instance);
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

void RenderingManager::CreateSwapchain()
{
    m_swapchain = std::make_unique<Swapchain>(m_logicalDevice->GetDevice(), m_deviceManager->Selected(),
                                              m_logicalDevice->GetDeviceQueueFamily(), m_surface, m_window);
}

void RenderingManager::CreateQueue()
{
    m_queue = std::make_unique<VulkanQueue>(*this, m_engine, m_logicalDevice->GetDevice(), m_swapchain->GetSwapchain(),
                                            m_logicalDevice->GetDeviceQueueFamily(), 0);
}

Swapchain& RenderingManager::GetSwapchain() const
{
    return *m_swapchain;
}

RenderingSync& RenderingManager::GetRenderingSync() const
{
    return *m_renderingSync;
}

VkInstance RenderingManager::GetInstance() const
{
    return m_instance;
}

Renderer& RenderingManager::GetRenderer() const
{
    return *m_renderer;
}

ImGuiRenderer& RenderingManager::GetImGuiRenderer() const
{
    return *m_imGuiRenderer;
}

VulkanAllocator& RenderingManager::GetAllocatorWrapper() const
{
    return *m_allocator;
}

void RenderingManager::OnWindowResize()
{
    m_queue->WaitIdle();
    vkDeviceWaitIdle(GetDevice());

    m_queue.reset();
    m_swapchain.reset();

    m_deviceManager->UpdateSurfaceCapabilities();

    CreateSwapchain();
    CreateQueue();

    FreeDepthResources();
    CreateDepthResources();

    m_renderer->RecreateCommandBuffers();

    m_imGuiRenderer->SetDisplaySize(m_window.GetDimensions());
}
