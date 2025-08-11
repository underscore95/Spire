#include "PipelineDescriptorSetsManager.h"

#include <unordered_map>
#include <unordered_set>
#include <libassert/assert.hpp>
#include <spdlog/spdlog.h>

#include "Engine/Rendering/Memory/BufferManager.h"
#include "Engine/Rendering/RenderingManager.h"
#include "Swapchain.h"
#include "Engine/Rendering/Memory/VulkanBuffer.h"
#include "Engine/Rendering/Memory/VulkanImage.h"

PipelineDescriptorSetsManager::PipelineDescriptorSetsManager(RenderingManager& renderingManager,
                                                             const std::vector<PipelineResourceInfo>& resources)
    : m_renderingManager(renderingManager),
      m_numSwapchainImages(renderingManager.GetSwapchain().GetNumImages()),
      m_resources(resources),
      m_device(m_renderingManager.GetDevice())
{
    for (const auto& resource : m_resources)
    {
        ASSERT(IsSupportedResourceType(resource.ResourceType));
        ASSERT(resource.ResourcePtrs.Buffers != nullptr); // todo is this best?
    }

    CreateDescriptorPool();
    CreateDescriptorSetLayout();
    AllocateDescriptorSets();
    UpdateDescriptorSets();
}

PipelineDescriptorSetsManager::~PipelineDescriptorSetsManager()
{
    vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
}

const std::vector<VkDescriptorSet>& PipelineDescriptorSetsManager::GetDescriptorSets() const
{
    return m_descriptorSets;
}

const VkDescriptorSetLayout* PipelineDescriptorSetsManager::GetPointerToFirstDescriptorSetLayout() const
{
    return &m_descriptorSetLayout;
}

void PipelineDescriptorSetsManager::CreateDescriptorPool()
{

}

void PipelineDescriptorSetsManager::CreateDescriptorSetLayout()
{

}

void PipelineDescriptorSetsManager::AllocateDescriptorSets()
{
    std::vector layouts(m_numSwapchainImages, m_descriptorSetLayout);

    VkDescriptorSetAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = m_descriptorPool,
        .descriptorSetCount = m_numSwapchainImages,
        .pSetLayouts = layouts.data()
    };

    m_descriptorSets.resize(m_numSwapchainImages);

    VkResult res = vkAllocateDescriptorSets(m_device, &allocInfo, m_descriptorSets.data());
    if (res != VK_SUCCESS)
    {
        spdlog::error("Failed to allocate descriptor sets");
    }
}

void PipelineDescriptorSetsManager::UpdateDescriptorSets() const
{
    std::vector<VkWriteDescriptorSet> writeDescriptorSets;
    std::vector<VkDescriptorBufferInfo> bufferInfos;
    std::vector<VkDescriptorImageInfo> imageInfos;
    bufferInfos.reserve(m_numSwapchainImages * m_resources.size());
    imageInfos.reserve(m_numSwapchainImages * m_resources.size());

    for (const auto& resource : m_resources)
    {
        DEBUG_ASSERT(resource.NumDescriptors > 0);
        if (resource.NumDescriptors > 1)
        {
            // texture arrays that need to be different per frame might not work
            DEBUG_ASSERT(resource.SameResourceForAllFrames);
            // only tested for texture arrays
            DEBUG_ASSERT(IsTextureSampler(resource.ResourceType));
        }

        for (glm::u32 imageIndex = 0; imageIndex < m_numSwapchainImages; imageIndex++)
        {
            // Common to all resources
            VkWriteDescriptorSet writeDescriptorSet = {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = m_descriptorSets[imageIndex],
                .dstBinding = resource.Binding,
                .dstArrayElement = 0,
                .descriptorCount = resource.NumDescriptors,
                .descriptorType = resource.ResourceType,
            };

            // Buffers
            if (IsBuffer(resource.ResourceType))
            {
                bufferInfos.push_back({
                    .buffer = resource.ResourcePtrs.Buffers[resource.SameResourceForAllFrames ? 0 : imageIndex].Buffer,
                    .offset = 0,
                    .range = VK_WHOLE_SIZE
                });
                writeDescriptorSet.pBufferInfo = &(bufferInfos[bufferInfos.size() - 1]);
            }

            // Textures
            else if (IsTextureSampler(resource.ResourceType))
            {
                for (glm::u32 texIndex = 0; texIndex < resource.NumDescriptors; texIndex++)
                {
                    const VulkanImage& tex =
                        resource.ResourcePtrs.Textures[texIndex + (resource.SameResourceForAllFrames ? 0 : imageIndex)];
                    imageInfos.push_back({
                        .sampler = tex.Sampler,
                        .imageView = tex.ImageView,
                        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                    });
                }
                writeDescriptorSet.pImageInfo = &(imageInfos[imageInfos.size() - resource.NumDescriptors]);
            }

            // ---
            else
                UNREACHABLE();

            writeDescriptorSets.push_back(writeDescriptorSet);
        }
    }

    vkUpdateDescriptorSets(m_device, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
}

bool PipelineDescriptorSetsManager::IsBuffer(VkDescriptorType resourceType) const
{
    switch (resourceType)
    {
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        return true;
    default:
        return false;
    }
}

bool PipelineDescriptorSetsManager::IsTextureSampler(VkDescriptorType resourceType) const
{
    switch (resourceType)
    {
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        return true;
    default:
        return false;
    }
}

bool PipelineDescriptorSetsManager::IsSupportedResourceType(VkDescriptorType resourceType) const
{
    return IsBuffer(resourceType) || IsTextureSampler(resourceType);
}
