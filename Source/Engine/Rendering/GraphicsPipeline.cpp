#include "GraphicsPipeline.h"
#include <glm/glm.hpp>
#include <libassert/assert.hpp>
#include <spdlog/spdlog.h>

#include "BufferManager.h"

GraphicsPipeline::GraphicsPipeline(VkDevice device, glm::uvec2 windowSize, VkRenderPass renderPass,
                                   VkShaderModule vertexShader, VkShaderModule fragmentShader,
                                   const VulkanBuffer &storageBufferForVertices, glm::u32 vertexBufferSize,
                                   glm::u32 numSwapchainImages) {
    m_device = device;

    CreateDescriptorSets(numSwapchainImages, storageBufferForVertices, vertexBufferSize);

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

    DEBUG_ASSERT(storageBufferForVertices.Buffer != nullptr);
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1, // the vertex buffer
        .pSetLayouts = &m_descriptorSetLayout
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
	vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);
    vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
	vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
    vkDestroyPipeline(m_device, m_pipeline, nullptr);
}

void GraphicsPipeline::BindTo(VkCommandBuffer commandBuffer, glm::u32 swapchainImageIndex) const {
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

    if (m_descriptorSets.size() > 0) {
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout,
                                0,  // firstSet
                                1,  // descriptorSetCount
                                &m_descriptorSets[swapchainImageIndex],
                                0,	// dynamicOffsetCount
                                nullptr);	// pDynamicOffsets
    }
}

void GraphicsPipeline::CreateDescriptorPool(glm::u32 numImages) {
    std::vector<VkDescriptorPoolSize> sizes;
    sizes.push_back({
        .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = numImages
    });

    VkDescriptorPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = 0,
        .maxSets = numImages,
        .poolSizeCount = static_cast<glm::u32>(sizes.size()),
        .pPoolSizes = sizes.data()
    };

    VkResult res = vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_descriptorPool);
    if (res != VK_SUCCESS) {
        spdlog::error("Failed to create descriptor pool");
    } else {
        spdlog::info("Created descriptor pool");
    }
}

void GraphicsPipeline::CreateDescriptorSets(glm::u32 numImages, const VulkanBuffer &storageBufferForVertices,
                                            glm::u32 vertexBufferSize) {
    CreateDescriptorPool(numImages);
    DEBUG_ASSERT(m_descriptorPool != VK_NULL_HANDLE);

    CreateDescriptorSetLayout();

    AllocateDescriptorSets(numImages);

    UpdateDescriptorSets(numImages, storageBufferForVertices, vertexBufferSize);
}

void GraphicsPipeline::CreateDescriptorSetLayout() {
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings;

    VkDescriptorSetLayoutBinding vertexShaderLayoutBinding_VertBuf = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
    };

    layoutBindings.push_back(vertexShaderLayoutBinding_VertBuf);

    VkDescriptorSetLayoutCreateInfo layoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0, // reserved - must be zero
        .bindingCount = static_cast<glm::u32>(layoutBindings.size()),
        .pBindings = layoutBindings.data()
    };

    VkResult res = vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &m_descriptorSetLayout);
    if (res != VK_SUCCESS) {
        spdlog::error("Failed to create descriptor set layout");
    }
}

void GraphicsPipeline::AllocateDescriptorSets(glm::u32 numImages) {
    std::vector layouts(numImages, m_descriptorSetLayout);

    VkDescriptorSetAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = m_descriptorPool,
        .descriptorSetCount = numImages,
        .pSetLayouts = layouts.data()
    };

    m_descriptorSets.resize(numImages);

    VkResult res = vkAllocateDescriptorSets(m_device, &allocInfo, m_descriptorSets.data());
    if (res != VK_SUCCESS) {
        spdlog::error("Failed to allocate descriptor sets");
    }
}

void GraphicsPipeline::UpdateDescriptorSets(glm::u32 numImages, const VulkanBuffer &storageBufferForVertices,
                                            glm::u32 vertexBufferSize) const {
    DEBUG_ASSERT(storageBufferForVertices.Buffer != VK_NULL_HANDLE);

    VkDescriptorBufferInfo bufferInfo_VB = {
        .buffer = storageBufferForVertices.Buffer,
        .offset = 0,
        .range = vertexBufferSize, // can also be VK_WHOLE_SIZE
    };

    std::vector<VkWriteDescriptorSet> writeDescriptorSet;

    for (glm::u32 i = 0; i < numImages; i++) {
        writeDescriptorSet.push_back(
            VkWriteDescriptorSet{
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = m_descriptorSets[i],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .pBufferInfo = &bufferInfo_VB
            }
        );
    }

    vkUpdateDescriptorSets(m_device, writeDescriptorSet.size(), writeDescriptorSet.data(), 0, nullptr);
}
