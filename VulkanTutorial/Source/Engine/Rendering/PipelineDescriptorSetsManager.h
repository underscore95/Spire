#pragma once
#include <vector>
#include <glm/fwd.hpp>
#include <vulkan/vulkan_core.h>

struct VulkanImage;
class VulkanBuffer;
class RenderingManager;

struct PipelineResourceInfo {
    VkDescriptorType ResourceType;
    glm::u32 Binding;
    VkShaderStageFlags Stages;

    bool SameResourceForAllImages;
    union ResourcePtr {
        void* Raw;
        VulkanBuffer *Buffers;
        VulkanImage *Textures;
    } ResourcePtrs;
};

class PipelineDescriptorSetsManager {
public:
    PipelineDescriptorSetsManager(RenderingManager &renderingManager,
                                  const std::vector<PipelineResourceInfo> &resources);
    ~PipelineDescriptorSetsManager();

public:
    const std::vector<VkDescriptorSet> &GetDescriptorSets() const;

    const VkDescriptorSetLayout *GetPointerToFirstDescriptorSetLayout() const;

private:
    void CreateDescriptorPool();

    void CreateDescriptorSetLayout();

    void AllocateDescriptorSets();

    void UpdateDescriptorSets() const;

    bool IsBuffer(VkDescriptorType resourceType) const;

    bool IsTextureSampler(VkDescriptorType resourceType) const;

    bool IsSupportedResourceType(VkDescriptorType resourceType) const;

private:
    RenderingManager &m_renderingManager;
    glm::u32 m_numSwapchainImages;
    const std::vector<PipelineResourceInfo> &m_resources;
    VkDevice m_device = VK_NULL_HANDLE;
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> m_descriptorSets;
};
