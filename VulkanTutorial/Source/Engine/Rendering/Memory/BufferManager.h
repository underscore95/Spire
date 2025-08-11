#pragma once
#include <vector>
#include <glm/fwd.hpp>
#include <vulkan/vulkan_core.h>

struct VulkanBuffer;
class RenderingManager;

class BufferManager
{
    friend class TextureManager;

public:
    explicit BufferManager(RenderingManager& renderingManager);

    ~BufferManager();

    VulkanBuffer CreateStorageBuffer(const void* elements, glm::u32 size, glm::u32 elementSize, bool isTransferSource = false);

    void DestroyBuffer(const VulkanBuffer& buffer);

    std::vector<VulkanBuffer> CreateUniformBuffers(size_t bufferSize, bool isTransferDest = false);

    void UpdateBuffer(const VulkanBuffer& buffer, const void* data, glm::u32 size) const;

private:
    void CopyBuffer(VkBuffer dest, VkBuffer src, VkDeviceSize size) const;

    VulkanBuffer CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

private:
    const glm::u32 INVALID_MEMORY_TYPE_INDEX = -1; // overflow
    RenderingManager& m_renderingManager;
    VkCommandBuffer m_copyCommandBuffer;
    glm::u32 m_numAllocatedBuffers = 0;
};
