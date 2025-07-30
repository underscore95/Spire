#pragma once
#include <vector>
#include <glm/fwd.hpp>
#include <vulkan/vulkan_core.h>

struct VulkanBuffer {
public:
    VkBuffer Buffer = nullptr;
    VkDeviceMemory DeviceMemory = nullptr;
    VkDeviceSize AllocationSize = 0;
};

class RenderingManager;

class BufferManager {
public:
    explicit BufferManager(RenderingManager &renderingManager);

    ~BufferManager();

    VulkanBuffer CreateStorageBufferForVertices(const void *pVertices, glm::u32 size) const;

    void DestroyBuffer(const VulkanBuffer &buffer) const;

    std::vector<VulkanBuffer> CreateUniformBuffers(size_t bufferSize) const;

    void UpdateBuffer(const VulkanBuffer &buffer, const void *data, glm::u32 size, glm::u32 offset = 0) const;

private:
    glm::u32 GetMemoryTypeIndex(glm::u32 memTypeBitsMask, VkMemoryPropertyFlags reqMemPropFlags) const;

    void CopyBuffer(VkBuffer dest, VkBuffer src, VkDeviceSize size) const;

    VulkanBuffer CreateBuffer(VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags Properties) const;

private:
    const glm::u32 INVALID_MEMORY_TYPE_INDEX = -1; // overflow
    RenderingManager &m_renderingManager;
    VkCommandBuffer m_copyCommandBuffer;
};
