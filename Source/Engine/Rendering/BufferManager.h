#pragma once
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

    VulkanBuffer CreateStorageBufferForVertices(const void *pVertices, size_t size) const;

    void DestroyBuffer(const VulkanBuffer &buffer) const;

private:
    glm::u32 GetMemoryTypeIndex(glm::u32 memTypeBitsMask, VkMemoryPropertyFlags reqMemPropFlags) const;

    void CopyBuffer(VkBuffer dest, VkBuffer src, VkDeviceSize size) const;

    VulkanBuffer CreateBuffer(VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags Properties) const;

private:
    const glm::u32 INVALID_MEMORY_TYPE_INDEX = -1; // overflow
    RenderingManager &m_renderingManager;
    VkCommandBuffer m_copyCommandBuffer;
};
