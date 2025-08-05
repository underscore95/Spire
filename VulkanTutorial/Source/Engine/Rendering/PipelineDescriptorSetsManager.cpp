#include "PipelineDescriptorSetsManager.h"

#include <unordered_map>
#include <unordered_set>
#include <libassert/assert.hpp>
#include <spdlog/spdlog.h>

#include "BufferManager.h"
#include "RenderingManager.h"
#include "Swapchain.h"
#include "TextureManager.h"
#include "VulkanBuffer.h"

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
    std::unordered_map<VkDescriptorType, glm::u32> numResourcesPerType;
    for (const auto& resource : m_resources)
    {
        numResourcesPerType[resource.ResourceType] += m_numSwapchainImages;
    }

    std::vector<VkDescriptorPoolSize> sizes;
    for (auto& [descriptorType, descriptorCount] : numResourcesPerType)
    {
        sizes.push_back({
            .type = descriptorType,
            .descriptorCount = descriptorCount
        });
    }

    VkDescriptorPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = 0,
        .maxSets = m_numSwapchainImages, // each image currently gets one set of descriptors
        .poolSizeCount = static_cast<glm::u32>(sizes.size()),
        .pPoolSizes = sizes.data()
    };

    VkResult res = vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_descriptorPool);
    if (res != VK_SUCCESS)
    {
        spdlog::error("Failed to create descriptor pool");
    }
    else
    {
        spdlog::info("Created descriptor pool");
    }
}

void PipelineDescriptorSetsManager::CreateDescriptorSetLayout()
{
    std::unordered_set<glm::u32> usedBindings;
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings;

    for (const auto& resource : m_resources)
    {
        // check bindings are unique
        DEBUG_ASSERT(!usedBindings.contains(resource.Binding));
        usedBindings.insert(resource.Binding);

        // create layout
        layoutBindings.push_back({
            .binding = resource.Binding,
            .descriptorType = resource.ResourceType,
            .descriptorCount = 1,
            .stageFlags = resource.Stages,
        });
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0, // reserved - must be zero
        .bindingCount = static_cast<glm::u32>(layoutBindings.size()),
        .pBindings = layoutBindings.data()
    };

    VkResult res = vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &m_descriptorSetLayout);
    if (res != VK_SUCCESS)
    {
        spdlog::error("Failed to create descriptor set layout");
    }
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
        for (glm::u32 imageIndex = 0; imageIndex < m_numSwapchainImages; imageIndex++)
        {
            // Common to all resources
            VkWriteDescriptorSet writeDescriptorSet = {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = m_descriptorSets[imageIndex],
                .dstBinding = resource.Binding,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = resource.ResourceType,
            };

            // Buffers
            if (IsBuffer(resource.ResourceType))
            {
                bufferInfos.push_back({
                    .buffer = resource.ResourcePtrs.Buffers[resource.SameResourceForAllImages ? 0 : imageIndex].Buffer,
                    .offset = 0,
                    .range = VK_WHOLE_SIZE
                });
                writeDescriptorSet.pBufferInfo = &(bufferInfos[bufferInfos.size() - 1]);
            }

            // Textures
            else if (IsTextureSampler(resource.ResourceType))
            {
                VulkanImage& tex = resource.ResourcePtrs.Textures[resource.SameResourceForAllImages ? 0 : imageIndex];
                imageInfos.push_back({
                    .sampler = tex.Sampler,
                    .imageView = tex.ImageView,
                    .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                });
                writeDescriptorSet.pImageInfo = &(imageInfos[imageInfos.size() - 1]);
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
