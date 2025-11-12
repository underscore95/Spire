#pragma once

namespace Spire {
    class RenderingManager;

    class CommandBufferVector {
    public:
        explicit CommandBufferVector(RenderingManager &renderingManager, glm::u32 numCommandBuffers);

        ~CommandBufferVector();

    public:
        [[nodiscard]] std::size_t Size() const;

        VkCommandBuffer &operator[](std::size_t i);

        [[nodiscard]] bool Empty() const;

    private:
        RenderingManager &m_renderingManager;
        std::vector<VkCommandBuffer> m_commandBuffers;
    };
} // Spire
