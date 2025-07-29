#pragma once

#include <vulkan/vulkan.hpp>
#include <glm/fwd.hpp>

struct GLFWwindow;

class GraphicsPipeline {
public:
    GraphicsPipeline(VkDevice device, glm::uvec2 windowSize, VkRenderPass renderPass,
                     VkShaderModule vertexShader, VkShaderModule fragmentShader);

    ~GraphicsPipeline();

public:
    void BindTo(VkCommandBuffer commandBuffer) const;

private:
    VkDevice m_device = nullptr;
    VkPipeline m_pipeline = nullptr;
    VkPipelineLayout m_pipelineLayout = nullptr;
};
