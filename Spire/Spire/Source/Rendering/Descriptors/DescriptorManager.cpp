#include "DescriptorManager.h"
#include "DescriptorPool.h"
#include "DescriptorSetsUpdater.h"
#include "Rendering/RenderingManager.h"
#include "Rendering/Core/Swapchain.h"
#include "Rendering/Memory/VulkanBuffer.h"
#include "Rendering/Memory/VulkanImage.h"
#include "Utils/Log.h"
#include "Utils/Timer.h"

namespace Spire {
    DescriptorManager::DescriptorManager(
        RenderingManager &renderingManager,
        const DescriptorSetLayoutList &layouts,
        VkPipelineBindPoint bindPoint
    ) : m_renderingManager(renderingManager),
        m_layouts(layouts),
        m_bindPoint(bindPoint) {
        m_pool = std::make_unique<DescriptorPool>(m_renderingManager);

        m_descriptorSets = m_pool->Allocate(layouts);
        m_rawLayouts = DescriptorSet::ToLayoutVector(m_descriptorSets);

        m_updater = std::make_unique<DescriptorSetsUpdater>(
            renderingManager.GetDevice(),
            static_cast<glm::u32>(m_descriptorSets.size()),
            m_descriptorSets.data()
        );
        m_updater->Update();
    }

    DescriptorManager::~DescriptorManager() {
        m_updater.reset();

        m_pool->Free(m_descriptorSets);
        m_pool.reset();
    }

    void DescriptorManager::CmdBind(VkCommandBuffer commandBuffer, glm::u32 currentSwapchainImage,
                                    VkPipelineLayout pipelineLayout, glm::u32 setIndex,
                                    glm::u32 shaderSetIndex) const {
        // this assert can be removed, i'm just curious why shader set index and set index would be different?
        assert(setIndex == shaderSetIndex);
        // currentSwapchainImage is allowed to be invalid as long as a per image set isn't being used
        assert(!m_layouts.IsSetPerImage(setIndex) || currentSwapchainImage < m_renderingManager.GetSwapchain().GetNumImages());
        glm::u32 offset = m_layouts.IsSetPerImage(setIndex) ? currentSwapchainImage : 0;
        m_descriptorSets[setIndex + offset].CmdBind(commandBuffer, pipelineLayout, shaderSetIndex, m_bindPoint);
    }

    void DescriptorManager::WriteDescriptor(glm::u32 setIndex, const Descriptor &descriptor) {
        VkDescriptorBufferInfo bufferInfo = {};

        assert(descriptor.Resources.size() == 1); // won't work probably if not 1
        assert(m_descriptorSets.size() > setIndex);
        const auto &set = m_descriptorSets[setIndex];

        VkWriteDescriptorSet write = {
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            nullptr,
            set.Handle,
            descriptor.Binding,
            0,
            1,
            descriptor.ResourceType,
            nullptr,
            nullptr,
            nullptr
        };

        // put resource info
        assert(m_updater->IsSupportedResourceType(descriptor.ResourceType));
        if (m_updater->IsBuffer(descriptor.ResourceType)) {
            bufferInfo = {
                descriptor.Resources[0].Buffer->Buffer,
                0,
                descriptor.Resources[0].Buffer->Size,
            };
            write.pBufferInfo = &bufferInfo;
        } else if (m_updater->IsImageSampler(descriptor.ResourceType)) {
            assert(false); // don't have a way to get image layout, need to get that to implement this
        } else {
            assert(false);
        }

        // now run it
        vkUpdateDescriptorSets(m_renderingManager.GetDevice(), 1, &write, 0, nullptr);
    }

    const std::vector<VkDescriptorSetLayout> &DescriptorManager::GetRawLayouts() const {
        return m_rawLayouts;
    }
}
