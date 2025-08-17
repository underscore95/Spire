#pragma once

#include <string>
#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include <glm/fwd.hpp>

#include "Core/VulkanVersion.h"

namespace Spire
{
    class VulkanAllocator;
    class ImGuiRenderer;
    class Renderer;
    class RenderingSync;
    class LogicalDevice;
    class VulkanDebugCallback;
    class Swapchain;
    struct VulkanImage;
    class TextureManager;
    class RenderingCommandManager;
    class Window;
    class RenderingDeviceManager;
    struct PhysicalDevice;
    class VulkanQueue;
    class BufferManager;
    class Engine;
    class DescriptorCreator;

    class RenderingManager
    {
        friend class Engine;

    public:
        explicit RenderingManager(Engine& engine, const std::string& applicationName, Window& window);

        ~RenderingManager();

    public:
        [[nodiscard]] bool IsValid() const;

        [[nodiscard]] RenderingCommandManager& GetCommandManager() const;

        [[nodiscard]] VulkanQueue& GetQueue() const;

        [[nodiscard]] glm::u32 GetQueueFamily() const;

        [[nodiscard]] VkDevice GetDevice() const;

        [[nodiscard]] const PhysicalDevice& GetPhysicalDevice() const;

        [[nodiscard]] BufferManager& GetBufferManager() const;

        [[nodiscard]] TextureManager& GetTextureManager() const;

        [[nodiscard]] const VulkanImage& GetDepthImage(glm::u32 imageIndex) const;

        [[nodiscard]] Swapchain& GetSwapchain() const;

        [[nodiscard]] RenderingSync& GetRenderingSync() const;

        [[nodiscard]] VkInstance GetInstance() const;

        [[nodiscard]] Renderer& GetRenderer() const;

        [[nodiscard]] ImGuiRenderer& GetImGuiRenderer() const;

        [[nodiscard]] VulkanAllocator& GetAllocatorWrapper() const;

        [[nodiscard]] DescriptorCreator& GetDescriptorCreator() const;

    private:
        void OnWindowResize();

        void GetInstanceVersion();

        void CreateDepthResources();

        void FreeDepthResources();

        void CreateInstance(const std::string& applicationName);

        void CreateSurface(const Window& window);

        void CreateSwapchain();

        void CreateQueue();

    private:
        Engine& m_engine;
        Window& m_window;
        VkInstance m_instance = VK_NULL_HANDLE;
        std::unique_ptr<VulkanDebugCallback> m_debugCallback = nullptr;
        VkSurfaceKHR m_surface = VK_NULL_HANDLE;
        std::unique_ptr<RenderingDeviceManager> m_deviceManager = nullptr;
        std::unique_ptr<LogicalDevice> m_logicalDevice = nullptr;
        std::unique_ptr<VulkanAllocator> m_allocator = nullptr;
        std::unique_ptr<Swapchain> m_swapchain = nullptr;
        std::unique_ptr<RenderingCommandManager> m_commandManager = nullptr;
        std::unique_ptr<VulkanQueue> m_queue = nullptr;
        std::unique_ptr<BufferManager> m_bufferManager = nullptr;
        std::unique_ptr<TextureManager> m_textureManager = nullptr;
        std::vector<VulkanImage> m_depthImages;
        VulkanVersion m_instanceVersion;
        std::unique_ptr<RenderingSync> m_renderingSync = nullptr;
        std::unique_ptr<Renderer> m_renderer = nullptr;
        std::unique_ptr<ImGuiRenderer> m_imGuiRenderer = nullptr;
        std::unique_ptr<DescriptorCreator> m_descriptorCreator = nullptr;
    };
}