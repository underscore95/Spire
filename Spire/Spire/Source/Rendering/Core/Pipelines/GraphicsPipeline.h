#pragma once

#include "Pipeline.h"

namespace Spire
{
    class DescriptorManager;
    class RenderingManager;
    struct GLFWwindow;
    struct VulkanBuffer;
    class DescriptorSet;

    class GraphicsPipeline final : public Pipeline
    {
    public:
        GraphicsPipeline(
            VkDevice device,
            VkShaderModule vertexShader,
            VkShaderModule fragmentShader,
            const DescriptorManager& descriptorManager,
            VkFormat colorFormat,
            VkFormat depthFormat,
            RenderingManager& renderingManager,
            glm::u32 pushConstantSize,
            bool renderWireframes
        );

        ~GraphicsPipeline() override;

    public:
        void CmdSetViewport(VkCommandBuffer commandBuffer, const VkViewport* viewport, const VkRect2D* scissor) const;
        void CmdSetViewportToWindowSize(VkCommandBuffer commandBuffer, glm::uvec2 windowDimensions) const;

        void CmdBindTo(VkCommandBuffer commandBuffer) const override;
    };
}