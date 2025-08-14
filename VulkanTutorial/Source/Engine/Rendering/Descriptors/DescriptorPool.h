#pragma once
#include <vector>
#include <vulkan/vulkan_core.h>
#include <glm/fwd.hpp>
#include "DescriptorSetLayout.h"
#include "DescriptorSet.h"

struct Descriptor;
class RenderingManager;

class DescriptorPool
{
public:
    explicit DescriptorPool(
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

    std::vector<DescriptorSet> Allocate(const DescriptorSetLayoutList& descriptorsLists);
    void Free(const std::vector<DescriptorSet>& descriptorSets);

private:
    glm::u32 m_numAllocatedSets = 0;
    RenderingManager& m_renderingManager;
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
};
