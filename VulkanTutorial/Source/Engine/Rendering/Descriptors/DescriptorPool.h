#pragma once
#include <vector>
#include <vulkan/vulkan_core.h>
#include <glm/fwd.hpp>

class DescriptorSetLayout;
class RenderingManager;
class DescriptorSet;

class DescriptorPool
{
public:
    DescriptorPool(
        RenderingManager& renderingManager,
        const std::vector<DescriptorSetLayout>& descriptorSets
    );
    ~DescriptorPool();

public:
    std::vector<DescriptorSet> Allocate(glm::u32 numDescriptorSets, const DescriptorSetLayout* descriptorSets);
    void Free(glm::u32 numDescriptorSets, const DescriptorSet* descriptorSets);

private:
    glm::u32 m_numAllocatedSets = 0;
    RenderingManager& m_renderingManager;
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
};
