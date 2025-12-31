#include "Pipeline.h"

#include "Utils/Log.h"

#include "Rendering/Descriptors/DescriptorManager.h"

namespace Spire {
    Pipeline::Pipeline(
        VkDevice device,
        const DescriptorManager &descriptorManager,
        RenderingManager &renderingManager,
        glm::u32 pushConstantSize
    ) : m_device(device),
        m_renderingManager(renderingManager),
        m_pushConstantSize(pushConstantSize),
        m_descriptorSetLayouts(descriptorManager.GetRawLayouts()) {
        CreatePipelineLayout();
    }

    Pipeline::~Pipeline() {
        vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
        assert(m_pipeline == VK_NULL_HANDLE); // child class should destroy it and set the handle to VK_NULL_HANDLE
    }

    void Pipeline::CmdSetPushConstants(VkCommandBuffer commandBuffer, const void *data, glm::u32 size,
                                       glm::u32 offset) const {
        assert(data != nullptr);
        assert(size + offset <= m_pushConstantSize);
        assert(size > 0);

        vkCmdPushConstants(
            commandBuffer,
            m_pipelineLayout,
            VK_SHADER_STAGE_ALL,
            offset,
            size,
            data
        );
    }

    VkPipelineLayout Pipeline::GetLayout() const {
        return m_pipelineLayout;
    }

    std::array<VkPipelineShaderStageCreateInfo, 2> Pipeline::CreateVertFragShaderInfo(VkShaderModule vertexShader,
                                                                                      VkShaderModule fragmentShader) const {
        return {
            CreateShaderInfo(vertexShader, VK_SHADER_STAGE_VERTEX_BIT),
            CreateShaderInfo(fragmentShader, VK_SHADER_STAGE_FRAGMENT_BIT)
        };
    }

    VkPipelineShaderStageCreateInfo Pipeline::CreateShaderInfo(VkShaderModule shader, VkShaderStageFlagBits stage,
                                                               const char *entryPoint) const {
        return {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = stage,
            .module = shader,
            .pName = entryPoint
        };
    }

    void Pipeline::CreatePipelineLayout() {
        VkPushConstantRange pushConstantRange = {
            .stageFlags = VK_SHADER_STAGE_ALL,
            .offset = 0,
            .size = m_pushConstantSize
        };

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = static_cast<glm::u32>(m_descriptorSetLayouts.size()),
            .pSetLayouts = m_descriptorSetLayouts.data(),
            .pushConstantRangeCount = 1,
            .pPushConstantRanges = &pushConstantRange
        };

        if (m_pushConstantSize == 0) {
            // having a 0 size push constant range isn't valid, so just don't add a push constant range
            // https://vulkan.lunarg.com/doc/view/1.4.321.1/windows/antora/spec/latest/chapters/descriptorsets.html#VUID-VkPushConstantRange-size-00296
            pipelineLayoutInfo.pushConstantRangeCount = 0;
            pipelineLayoutInfo.pPushConstantRanges = nullptr;
        }

        VkResult res = vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout);
        if (res != VK_SUCCESS) {
            error("Failed to create vulkan pipeline layout");
        }
    }
}
