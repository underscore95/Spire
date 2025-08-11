#pragma once
#include <vector>
#include <vulkan/vulkan_core.h>

class DescriptorSet;
class RenderingManager;

class DescriptorPool
{
public:
    DescriptorPool(
        RenderingManager& renderingManager,
        const std::vector<DescriptorSet>& descriptorSets
    );
    ~DescriptorPool();

private:
    RenderingManager& m_renderingManager;
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
};
