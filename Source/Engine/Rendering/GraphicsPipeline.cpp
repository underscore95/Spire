#include "GraphicsPipeline.h"
#include <glm/glm.hpp>
#include <libassert/assert.hpp>
#include <spdlog/spdlog.h>

#include "BufferManager.h"
#include "PipelineDescriptorSetsManager.h"

GraphicsPipeline::GraphicsPipeline(VkDevice device,
                                   glm::uvec2 windowSize,
                                   VkRenderPass renderPass,
                                   VkShaderModule vertexShader,
                                   VkShaderModule fragmentShader,
                                   std::unique_ptr<PipelineDescriptorSetsManager>
                                   descriptorSetsManager)
    : m_device(device),
      m_descriptorSetsManager(std::move(descriptorSetsManager)) {
    constexpr glm::u32 NUM_SHADER_STAGES = 2;
    VkPipelineShaderStageCreateInfo shaderStageCreateInfo[NUM_SHADER_STAGES] = {
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertexShader,
            .pName = "main",
        },
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragmentShader,
            .pName = "main"
        }
    };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
    };

    VkPipelineInputAssemblyStateCreateInfo pipelineIACreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE
    };

    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(windowSize.x),
        .height = static_cast<float>(windowSize.y),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    VkRect2D scissor{
        .offset = {
            .x = 0,
            .y = 0,
        },
        .extent = {
            .width = windowSize.x,
            .height = windowSize.y,
        }
    };

    VkPipelineViewportStateCreateInfo viewportCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor
    };

    VkPipelineRasterizationStateCreateInfo rastCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_NONE,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .lineWidth = 1.0f
    };

    VkPipelineMultisampleStateCreateInfo pipelineMultisampleCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0f
    };

    VkPipelineColorBlendAttachmentState blendAttachState = {
        .blendEnable = VK_FALSE,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                          VK_COLOR_COMPONENT_A_BIT
    };

    VkPipelineColorBlendStateCreateInfo blendCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &blendAttachState
    };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = m_descriptorSetsManager->GetPointerToFirstDescriptorSetLayout()
    };

    VkResult res = vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout);
    if (res != VK_SUCCESS) {
        spdlog::error("Failed to create graphics pipeline layout");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = NUM_SHADER_STAGES,
        .pStages = &shaderStageCreateInfo[0],
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &pipelineIACreateInfo,
        .pViewportState = &viewportCreateInfo,
        .pRasterizationState = &rastCreateInfo,
        .pMultisampleState = &pipelineMultisampleCreateInfo,
        .pColorBlendState = &blendCreateInfo,
        .layout = m_pipelineLayout,
        .renderPass = renderPass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1
    };

    res = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline);
    if (res != VK_SUCCESS) {
        spdlog::error("Failed to create graphics pipeline layout");
    } else {
        spdlog::info("Graphics pipeline created");
    }
}

GraphicsPipeline::~GraphicsPipeline() {
    vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
    m_descriptorSetsManager.reset();
    vkDestroyPipeline(m_device, m_pipeline, nullptr);
}

void GraphicsPipeline::BindTo(VkCommandBuffer commandBuffer, glm::u32 swapchainImageIndex) const {
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

    const auto &descriptorSets = m_descriptorSetsManager->GetDescriptorSets();
    if (descriptorSets.size() > 0) {
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout,
                                0, // firstSet
                                1, // descriptorSetCount
                                &descriptorSets[swapchainImageIndex],
                                0, // dynamicOffsetCount
                                nullptr); // pDynamicOffsets
    }
}
