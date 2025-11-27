#include "GraphicsPipeline.h"
#include "pch.h"
#include "Utils/Log.h"
#include "Rendering/Memory/BufferManager.h"
#include "../RenderingCommandManager.h"

namespace Spire
{
    GraphicsPipeline::GraphicsPipeline(
        VkDevice device,
        VkShaderModule vertexShader,
        VkShaderModule fragmentShader,
        const DescriptorManager& descriptorManager,
        VkFormat colorFormat,
        VkFormat depthFormat,
        RenderingManager& renderingManager,
        glm::u32 pushConstantSize)
        : Pipeline(device, descriptorManager, renderingManager, pushConstantSize)
    {
        // pipeline
        std::array<VkPipelineShaderStageCreateInfo, 2> shaderStageCreateInfo = CreateVertFragShaderInfo(
            vertexShader, fragmentShader);

        VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
        };

        VkPipelineInputAssemblyStateCreateInfo pipelineIACreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE
        };

        VkPipelineViewportStateCreateInfo viewportCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .pViewports = nullptr,
            .scissorCount = 1,
            .pScissors = nullptr
        };

        VkDynamicState dynamicStates[] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = 2;
        dynamicState.pDynamicStates = dynamicStates;

        VkPipelineRasterizationStateCreateInfo rastCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_BACK_BIT,
            .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
            .lineWidth = 1.0f
        };

        VkPipelineMultisampleStateCreateInfo pipelineMultisampleCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,
            .minSampleShading = 1.0f
        };

        VkPipelineDepthStencilStateCreateInfo depthStencilState = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .depthTestEnable = VK_TRUE,
            .depthWriteEnable = VK_TRUE,
            .depthCompareOp = VK_COMPARE_OP_LESS,
            .depthBoundsTestEnable = VK_FALSE,
            .stencilTestEnable = VK_FALSE,
            .front = {},
            .back = {},
            .minDepthBounds = 0.0f,
            .maxDepthBounds = 1.0f
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

        VkPipelineRenderingCreateInfo renderingInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
            .pNext = nullptr,
            .viewMask = 0,
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &colorFormat,
            .depthAttachmentFormat = depthFormat,
            .stencilAttachmentFormat = VK_FORMAT_UNDEFINED
        };

        VkGraphicsPipelineCreateInfo pipelineInfo = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &renderingInfo,
            .stageCount = shaderStageCreateInfo.size(),
            .pStages = shaderStageCreateInfo.data(),
            .pVertexInputState = &vertexInputInfo,
            .pInputAssemblyState = &pipelineIACreateInfo,
            .pViewportState = &viewportCreateInfo,
            .pRasterizationState = &rastCreateInfo,
            .pMultisampleState = &pipelineMultisampleCreateInfo,
            .pDepthStencilState = &depthStencilState,
            .pColorBlendState = &blendCreateInfo,
            .pDynamicState = &dynamicState,
            .layout = m_pipelineLayout,
            .renderPass = VK_NULL_HANDLE,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = -1
        };

        VkResult res = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline);
        if (res != VK_SUCCESS)
        {
            error("Failed to create graphics pipeline layout");
        }
    }

    GraphicsPipeline::~GraphicsPipeline()
    {
        vkDestroyPipeline(m_device, m_pipeline, nullptr);
        m_pipeline = VK_NULL_HANDLE;
    }

    void GraphicsPipeline::CmdSetViewport(VkCommandBuffer commandBuffer, const VkViewport* viewport,
                                          const VkRect2D* scissor) const
    {
        if (viewport) vkCmdSetViewport(commandBuffer, 0, 1, viewport);
        if (scissor) vkCmdSetScissor(commandBuffer, 0, 1, scissor);
    }

    void GraphicsPipeline::CmdSetViewportToWindowSize(VkCommandBuffer commandBuffer, glm::uvec2 windowDimensions) const
    {
        VkViewport viewport = {
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(windowDimensions.x),
            .height = static_cast<float>(windowDimensions.y),
            .minDepth = 0.0f,
            .maxDepth = 1.0f
        };

        VkRect2D scissor{
            .offset = {
                .x = 0,
                .y = 0,
            },
            .extent = {
                .width = windowDimensions.x,
                .height = windowDimensions.y,
            }
        };

        CmdSetViewport(commandBuffer, &viewport, &scissor);
    }

    void GraphicsPipeline::CmdBindTo(VkCommandBuffer commandBuffer) const
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
    }
}