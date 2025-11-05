#pragma once

#include "pch.h"

namespace Spire
{
    class Window;
    class RenderingManager;

    class ImGuiRenderer
    {
    public:
        explicit ImGuiRenderer(
            RenderingManager& renderingManager,
            const Window& window
        );
        ~ImGuiRenderer();

    public:
        void SetDisplaySize(glm::uvec2 displaySize) const;

        VkCommandBuffer PrepareCommandBuffer(glm::u32 imageIndex) const;

    private:
        void CreateDescriptorPool();

        void InitImGUI();

        static void CheckVkResult(VkResult res);

    private:
        RenderingManager& m_renderingManager;
        const Window& m_window;
        std::vector<VkCommandBuffer> m_commandBuffers;
        VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
    };
}