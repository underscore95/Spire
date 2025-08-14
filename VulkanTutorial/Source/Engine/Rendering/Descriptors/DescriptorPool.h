#pragma once
#include <vector>
#include <vulkan/vulkan_core.h>
#include <glm/fwd.hpp>

struct Descriptor;
class DescriptorSetLayout;
class RenderingManager;

class DescriptorPool
{
public:
    DescriptorPool(
        RenderingManager& renderingManager
    );
    ~DescriptorPool();

public:
    // represents a single descriptor set
    struct Allocation
    {
        glm::u32 NumDescriptors;
        Descriptor* Descriptors;
    };

    std::vector<DescriptorSetLayout> Allocate(const std::vector<std::vector<Descriptor>>& descriptorsLists);
    void Free(const std::vector<DescriptorSetLayout>& descriptorSets);

private:
    glm::u32 m_numAllocatedSets = 0;
    RenderingManager& m_renderingManager;
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
};
