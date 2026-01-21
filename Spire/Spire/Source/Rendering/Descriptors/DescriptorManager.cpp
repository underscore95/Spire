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
        const DescriptorSetLayoutList &layouts
    ) : m_renderingManager(renderingManager),
        m_layouts(layouts) {
        m_pool = std::make_unique<DescriptorPool>(m_renderingManager);

        m_descriptorSets = m_pool->Allocate(layouts);
        m_rawLayouts = DescriptorSet::ToLayoutVector(m_descriptorSets);

        m_updater = std::make_unique<DescriptorSetsUpdater>(
            renderingManager.GetDevice(),
            static_cast<glm::u32>(m_descriptorSets.size()),
            m_descriptorSets.data()
        );
        m_updater->Update();

        // Set debug names
#ifndef NDEBUG
        vkSetDebugUtilsObjectNameEXT_fn = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(
            vkGetInstanceProcAddr(m_renderingManager.GetInstance(), "vkSetDebugUtilsObjectNameEXT")
        );
        assert(vkSetDebugUtilsObjectNameEXT_fn);


        for (const DescriptorSetLayout &set : layouts.GetDescriptorSets()) {
            for (const Descriptor &descriptor : set) {
                SetResourceDebugName(descriptor);
            }
        }
#endif
    }

    DescriptorManager::~DescriptorManager() {
        m_updater.reset();

        m_pool->Free(m_descriptorSets);
        m_pool.reset();
    }

    void DescriptorManager::CmdBind(VkCommandBuffer commandBuffer, glm::u32 currentSwapchainImage,
                                    VkPipelineLayout pipelineLayout, glm::u32 setIndex,
                                    glm::u32 shaderSetIndex) const {
        assert(setIndex == shaderSetIndex); // this assert can be removed, i'm just curious why shader set index and set index would be different?
        glm::u32 offset = m_layouts.IsSetPerImage(setIndex) ? currentSwapchainImage : 0;
        m_descriptorSets[setIndex + offset].CmdBind(commandBuffer, pipelineLayout, shaderSetIndex);
    }

    void DescriptorManager::WriteDescriptor(glm::u32 setIndex, const Descriptor &descriptor) const {
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

#ifndef NDEBUG
        SetResourceDebugName(descriptor);
#endif
    }

    const std::vector<VkDescriptorSetLayout> &DescriptorManager::GetRawLayouts() const {
        return m_rawLayouts;
    }

    void DescriptorManager::SetResourceDebugName(const Descriptor &descriptor) const {
#ifndef NDEBUG
        if (!DescriptorSetsUpdater::IsSupportedResourceType(descriptor.ResourceType)) return;
        assert(vkSetDebugUtilsObjectNameEXT_fn);

        for (std::size_t i = 0; i < descriptor.Resources.size(); i++) {
            std::string baseName = descriptor.DebugName ;
            if (descriptor.Resources.size() > 1) baseName += std::format("[{}]", i); // index

            if (DescriptorSetsUpdater::IsBuffer(descriptor.ResourceType)) {
                VkDebugUtilsObjectNameInfoEXT info{};
                info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
                info.objectType = VK_OBJECT_TYPE_BUFFER;
                info.objectHandle =
                        reinterpret_cast<uint64_t>(descriptor.Resources[i].Buffer->Buffer);
                info.pObjectName = baseName.c_str();

                vkSetDebugUtilsObjectNameEXT_fn(m_renderingManager.GetDevice(), &info);
            } else {
                // VulkanImage contains an Image, ImageView, and Sampler
                // We need to name all 3
                {
                    VkDebugUtilsObjectNameInfoEXT info{};
                    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
                    info.objectType = VK_OBJECT_TYPE_IMAGE;
                    info.objectHandle =
                            reinterpret_cast<uint64_t>(descriptor.Resources[i].Image->Image);
                    std::string name = baseName + ".Image";
                    info.pObjectName = name.c_str();

                    vkSetDebugUtilsObjectNameEXT_fn(m_renderingManager.GetDevice(), &info);
                } {
                    VkDebugUtilsObjectNameInfoEXT info{};
                    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
                    info.objectType = VK_OBJECT_TYPE_IMAGE_VIEW;
                    info.objectHandle =
                            reinterpret_cast<uint64_t>(descriptor.Resources[i].Image->ImageView);
                    std::string name = baseName + ".ImageView";
                    info.pObjectName = name.c_str();

                    vkSetDebugUtilsObjectNameEXT_fn(m_renderingManager.GetDevice(), &info);
                } {
                    VkDebugUtilsObjectNameInfoEXT info{};
                    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
                    info.objectType = VK_OBJECT_TYPE_SAMPLER;
                    info.objectHandle =
                            reinterpret_cast<uint64_t>(descriptor.Resources[i].Image->Sampler);
                    std::string name = baseName + ".Sampler";
                    info.pObjectName = name.c_str();

                    vkSetDebugUtilsObjectNameEXT_fn(m_renderingManager.GetDevice(), &info);
                }
            }
        }
#endif
    }
}
