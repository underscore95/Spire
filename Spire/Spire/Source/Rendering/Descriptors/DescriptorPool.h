#pragma once

#include "pch.h"
#include "DescriptorSetLayoutList.h"
#include "DescriptorSet.h"

namespace Spire {
    struct Descriptor;
    class RenderingManager;

    class DescriptorPool {
    public:
        struct Settings {
            glm::u32 UniformBuffers = 100;
            glm::u32 StorageBuffers = 100;
            glm::u32 CombinedImageSamplers = 100;
        };

    public:
        explicit DescriptorPool(
            RenderingManager &renderingManager,
            Settings settings
        );

        ~DescriptorPool();

    public:
        // represents a single descriptor set
        struct Allocation {
            glm::u32 NumDescriptors;
            Descriptor *Descriptors;
        };

        std::vector<DescriptorSet> Allocate(const DescriptorSetLayoutList &descriptorsLists);

        void Free(const std::vector<DescriptorSet> &descriptorSets);

    private:
        glm::u32 m_numAllocatedSets = 0;
        RenderingManager &m_renderingManager;
        VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
    };
}
