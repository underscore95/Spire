#include "DescriptorSetsUpdater.h"

#include "Rendering/Memory/VulkanImage.h"
#include "Rendering/Memory/VulkanBuffer.h"
#include "DescriptorSet.h"

namespace Spire {
    DescriptorSetsUpdater::DescriptorSetsUpdater(
        VkDevice device,
        glm::u32 numDescriptorSets,
        const DescriptorSet *descriptorSets
    ) : m_device(device) {
        // Constructor just prepares the write descriptor sets but doesn't update anything
        assert(numDescriptorSets > 0);
        assert(descriptorSets != nullptr);

        // Reserve space since we are using memory addresses so can't have resizing
        glm::u32 totalDescriptorCount = 0;
        glm::u32 totalWriteDescriptorsRequired = 0;
        for (glm::u32 i = 0; i < numDescriptorSets; i++) {
            const DescriptorSet &descriptorSet = descriptorSets[i];
            for (const Descriptor &descriptor : descriptorSet.Descriptors) {
                totalWriteDescriptorsRequired++;
                totalDescriptorCount += descriptor.Resources.size();
            }
        }
        m_writeDescriptorSets.reserve(totalWriteDescriptorsRequired);
        m_descriptorInfos.reserve(totalDescriptorCount);

        // Create the write descriptors
        for (glm::u32 i = 0; i < numDescriptorSets; i++) {
            const DescriptorSet &descriptorSet = descriptorSets[i];
            for (const Descriptor &descriptor : descriptorSet.Descriptors) {
                assert(!descriptor.Resources.empty());

                VkWriteDescriptorSet writeDescriptorSet = {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .dstSet = descriptorSet.Handle,
                    .dstBinding = descriptor.Binding,
                    .dstArrayElement = 0,
                    .descriptorCount = static_cast<glm::u32>(descriptor.Resources.size()),
                    .descriptorType = descriptor.ResourceType,
                };

                // Descriptor infos
                glm::u32 firstDescriptorInfoIndex = m_descriptorInfos.size();

                for (glm::u32 resourceIndex = 0; resourceIndex < descriptor.Resources.size(); resourceIndex++) {
                    DescriptorInfo descriptorInfo = {};

                    // Buffers
                    if (IsBuffer(descriptor.ResourceType)) {
                        descriptorInfo.BufferInfo = {
                            .buffer = descriptor.Resources[resourceIndex].Buffer->Buffer,
                            .offset = 0,
                            .range = VK_WHOLE_SIZE
                        };
                    }

                    // Images
                    else if (IsImageSampler(descriptor.ResourceType)) {
                        const VulkanImage &texture = *descriptor.Resources[resourceIndex].Image;
                        descriptorInfo.ImageInfo = {
                            .sampler = texture.Sampler,
                            .imageView = texture.ImageView,
                            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                        };
                    }

                    // Unsupported resource type
                    else
                        assert(false);

                    m_descriptorInfos.push_back(descriptorInfo);
                }

                // Point to first descriptor info
                if (IsBuffer(descriptor.ResourceType)) {
                    writeDescriptorSet.pBufferInfo = &(m_descriptorInfos[firstDescriptorInfoIndex].BufferInfo);
                } else if (IsImageSampler(descriptor.ResourceType)) {
                    writeDescriptorSet.pImageInfo = &(m_descriptorInfos[firstDescriptorInfoIndex].ImageInfo);
                }

                // Unsupported resource type
                else
                    assert(false);

                m_writeDescriptorSets.push_back(writeDescriptorSet);
            }
        }

        assert(m_writeDescriptorSets.size() == totalWriteDescriptorsRequired);
        assert(m_descriptorInfos.size() == totalDescriptorCount);
    }

    DescriptorSetsUpdater::~DescriptorSetsUpdater() {
        assert(m_hasUpdatedAtLeastOnce);
        // You haven't done anything with this you need to call Update() to actually update the descriptors
    }

    void DescriptorSetsUpdater::Update() {
        assert(!m_writeDescriptorSets.empty());
        m_hasUpdatedAtLeastOnce = true;

        vkUpdateDescriptorSets(m_device, m_writeDescriptorSets.size(), m_writeDescriptorSets.data(), 0, nullptr);
    }

    bool DescriptorSetsUpdater::IsBuffer(VkDescriptorType resourceType) const {
        switch (resourceType) {
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                return true;
            default:
                return false;
        }
    }

    bool DescriptorSetsUpdater::IsImageSampler(VkDescriptorType resourceType) const {
        switch (resourceType) {
            case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                return true;
            default:
                return false;
        }
    }

    bool DescriptorSetsUpdater::IsSupportedResourceType(VkDescriptorType resourceType) const {
        return IsBuffer(resourceType) || IsImageSampler(resourceType);
    }
}
