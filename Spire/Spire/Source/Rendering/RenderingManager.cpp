// ReSharper disable CppDFATimeOver

#include <glm/glm.hpp>
#include <vector>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "Utils/Log.h"
#include "Core/RenderingCommandManager.h"
#include "Core/RenderingDeviceManager.h"
#include "Core/VulkanQueue.h"
#include "Core/Engine.h"
#include "Core/VulkanUtils.h"
#include "Window/Window.h"
#include "Memory/BufferManager.h"
#include "Renderers/ImGuiRenderer.h"
#include "Core/LogicalDevice.h"
#include "Renderers/Renderer.h"
#include "Core/RenderingSync.h"
#include "Core/Swapchain.h"
#include "Memory/ImageManager.h"
#include "Memory/VulkanAllocator.h"
#include "Core/VulkanDebugCallback.h"
#include "Descriptors/DescriptorCreator.h"
#include "Memory/VulkanImage.h"
#include "RenderingManager.h"

namespace Spire {
    RenderingManager::RenderingManager(Engine &engine,
                                       const std::string &applicationName,
                                       Window &window,
                                       glm::u32 maximumSwapchainImages)
        : m_engine(engine),
          m_window(window),
          m_maximumSwapchainImages(maximumSwapchainImages) {
        info("Initializing RenderingManager...");

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
        m_descriptorCreator = std::make_unique<DescriptorCreator>(m_swapchain->GetNumImages());

        m_commandManager = std::make_unique<RenderingCommandManager>(m_logicalDevice->GetDeviceQueueFamily(),
                                                                     m_logicalDevice->GetDevice());

        CreateQueue();

        m_bufferManager = std::make_unique<BufferManager>(*this);
        m_imageManager = std::make_unique<ImageManager>(*this);

        CreateDepthResources();

        m_renderer = std::make_unique<Renderer>(*this, window);
        m_imGuiRenderer = std::make_unique<ImGuiRenderer>(*this, window);

        info("Initialized RenderingManager!");
    }

    RenderingManager::~RenderingManager() {
        info("Destroying RenderingManager...");

        m_imGuiRenderer.reset();
        m_renderer.reset();

        FreeDepthResources();

        m_imageManager.reset();
        m_bufferManager.reset();

        m_queue.reset();

        m_commandManager.reset();

        m_swapchain.reset();

        m_allocator.reset();

        m_logicalDevice.reset();
        m_deviceManager.reset();

        if (m_surface) {
            vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
            info("Destroyed Vulkan surface instance");
        }

        m_descriptorCreator.reset();

        m_debugCallback.reset();

        if (m_instance) {
            vkDestroyInstance(m_instance, nullptr);
            info("Destroyed Vulkan instance");
        }

        info("Destroyed RenderingManager!");
    }

    bool RenderingManager::IsValid() const {
        return m_instance &&
               m_surface &&
               m_deviceManager &&
               m_logicalDevice->IsValid() &&
               m_swapchain->IsValid();
    }

    RenderingCommandManager &RenderingManager::GetCommandManager() const {
        return *m_commandManager;
    }

    VulkanQueue &RenderingManager::GetQueue() const {
        return *m_queue;
    }

    glm::u32 RenderingManager::GetQueueFamily() const {
        return m_logicalDevice->GetDeviceQueueFamily();
    }

    VkDevice RenderingManager::GetDevice() const {
        return m_logicalDevice->GetDevice();
    }

    const PhysicalDevice &RenderingManager::GetPhysicalDevice() const {
        return m_deviceManager->Selected();
    }

    BufferManager &RenderingManager::GetBufferManager() const {
        return *m_bufferManager;
    }

    ImageManager &RenderingManager::GetImageManager() const {
        return *m_imageManager;
    }

    const VulkanImage &RenderingManager::GetDepthImage(glm::u32 imageIndex) const {
        assert(imageIndex < m_depthImages.size());
        return m_depthImages[imageIndex];
    }

    void RenderingManager::GetInstanceVersion() {
        VkResult res = vkEnumerateInstanceVersion(&m_instanceVersion.RawVersion);
        if (res != VK_SUCCESS) {
            error("Failed to get vulkan instance version");
        }

        m_instanceVersion.Variant = VK_API_VERSION_VARIANT(m_instanceVersion.RawVersion);
        m_instanceVersion.Major = VK_API_VERSION_MAJOR(m_instanceVersion.RawVersion);
        m_instanceVersion.Minor = VK_API_VERSION_MINOR(m_instanceVersion.RawVersion);
        m_instanceVersion.Patch = VK_API_VERSION_PATCH(m_instanceVersion.RawVersion);

        info("Vulkan loader supports version {}.{}.{}.{}", m_instanceVersion.Variant, m_instanceVersion.Major,
             m_instanceVersion.Minor, m_instanceVersion.Patch);
    }

    void RenderingManager::CreateDepthResources() {
        m_depthImages.resize(m_swapchain->GetNumImages());

        VkFormat depthFormat = m_deviceManager->Selected().DepthFormat;

        for (int i = 0; i < m_depthImages.size(); i++) {
            VkImageUsageFlagBits usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            VkMemoryPropertyFlagBits propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            m_imageManager->CreateImage(m_depthImages[i], m_window.GetDimensions(), usage, propertyFlags, depthFormat);

            m_imageManager->TransitionImageLayout(m_depthImages[i].Image, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED,
                                                  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

            m_depthImages[i].ImageView = m_imageManager->CreateImageView(m_depthImages[i].Image, depthFormat,
                                                                         VK_IMAGE_ASPECT_DEPTH_BIT);
        }

        info("Created {} depth images", m_depthImages.size());
    }

    void RenderingManager::FreeDepthResources() {
        for (const auto &depthImage : m_depthImages) {
            m_imageManager->DestroyImage(depthImage);
        }
        m_depthImages.clear();
    }

    void RenderingManager::CreateInstance(const std::string &applicationName) {
        std::vector<const char *> layers = {
#ifndef NDEBUG
            "VK_LAYER_KHRONOS_validation"
#endif
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
#ifndef NDEBUG
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
        };

        assert(
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

        std::array<VkValidationFeatureEnableEXT, 1> enables = {
            VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT
        };

        VkValidationFeaturesEXT validationFeatures = {
            .sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
            .pNext = VK_NULL_HANDLE,
            .enabledValidationFeatureCount = enables.size(),
            .pEnabledValidationFeatures = enables.data(),
            .disabledValidationFeatureCount = 0,
            .pDisabledValidationFeatures = VK_NULL_HANDLE
        };

        VkInstanceCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = &validationFeatures,
            .flags = 0, // reserved for future use. Must be zero
            .pApplicationInfo = &AppInfo,
            .enabledLayerCount = static_cast<glm::u32>(layers.size()),
            .ppEnabledLayerNames = layers.data(),
            .enabledExtensionCount = static_cast<glm::u32>(Extensions.size()),
            .ppEnabledExtensionNames = Extensions.data()
        };

        VkResult res = vkCreateInstance(&createInfo, nullptr, &m_instance);
        if (res != VK_SUCCESS) {
            error("Failed to create Vulkan instance: {}", static_cast<int>(res));
            return;
        }

        info("Created Vulkan instance");
    }

    void RenderingManager::CreateSurface(const Window &window) {
        VkResult result = glfwCreateWindowSurface(m_instance, window.GLFWWindow(), nullptr, &m_surface);
        if (result != VK_SUCCESS) {
            error("Failed to create Vulkan window surface, result: {}", static_cast<int>(result));
        } else {
            info("Created Vulkan window surface");
        }
    }

    void RenderingManager::CreateSwapchain() {
        m_swapchain = std::make_unique<Swapchain>(m_logicalDevice->GetDevice(), m_deviceManager->Selected(),
                                                  m_logicalDevice->GetDeviceQueueFamily(), m_surface, m_window, m_maximumSwapchainImages);
    }

    void RenderingManager::CreateQueue() {
        m_queue = std::make_unique<VulkanQueue>(*this, m_engine, m_logicalDevice->GetDevice(), m_swapchain->GetSwapchain(),
                                                m_logicalDevice->GetDeviceQueueFamily(), 0);
    }

    Swapchain &RenderingManager::GetSwapchain() const {
        return *m_swapchain;
    }

    RenderingSync &RenderingManager::GetRenderingSync() const {
        return *m_renderingSync;
    }

    VkInstance RenderingManager::GetInstance() const {
        return m_instance;
    }

    Renderer &RenderingManager::GetRenderer() const {
        return *m_renderer;
    }

    ImGuiRenderer &RenderingManager::GetImGuiRenderer() const {
        return *m_imGuiRenderer;
    }

    VulkanAllocator &RenderingManager::GetAllocatorWrapper() const {
        return *m_allocator;
    }

    DescriptorCreator &RenderingManager::GetDescriptorCreator() const {
        return *m_descriptorCreator;
    }

    void RenderingManager::OnWindowResize() {
        m_queue->WaitIdle();
        vkDeviceWaitIdle(GetDevice());

        m_queue.reset();
        m_swapchain.reset();

        m_deviceManager->UpdateSurfaceCapabilities();

        CreateSwapchain();
        if (!m_swapchain || !m_swapchain->IsValid()) error("Failed to recreate swapchain {}", static_cast<const void *>(m_swapchain.get()));
        CreateQueue();

        FreeDepthResources();
        CreateDepthResources();

        m_renderer->RecreateCommandBuffers();

        m_imGuiRenderer->SetDisplaySize(m_window.GetDimensions());
    }
}
