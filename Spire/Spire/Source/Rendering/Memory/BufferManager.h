#pragma once
#include "pch.h"

namespace Spire {
    class PerImageBuffer;
    struct VulkanBuffer;
    class RenderingManager;

    class BufferManager {
        friend class ImageManager;

    public:
        explicit BufferManager(RenderingManager &renderingManager);

        ~BufferManager();

        // Create an index buffer, indexTypeSize determines type of indices, either sizeof(std::uint32_t) or sizeof(std::uint16_t) or sizeof(std::uint8_t)
        VulkanBuffer CreateIndexBuffer(glm::u32 indexTypeSize, const void *indices, glm::u32 numIndices);

        // elements can be nullptr which means that initial data is undefined
        VulkanBuffer CreateStorageBuffer(const void *elements, glm::u32 size, glm::u32 elementSize,
                                         bool isTransferSource = false);

        void DestroyBuffer(const VulkanBuffer &buffer);

        std::unique_ptr<PerImageBuffer> CreateUniformBuffers(size_t bufferSize, bool isTransferDest = false);

        std::unique_ptr<PerImageBuffer> CreateStorageBuffers(size_t bufferSize, size_t numElements, const void *data);

        void UpdateBuffer(const VulkanBuffer &buffer, const void *data, glm::u32 size, glm::u32 offset = 0) const;

    public:
        static bool HasBufferManagerBeenDestroyed();

    private:
        void CopyBuffer(VkBuffer dest, VkBuffer src, VkDeviceSize size) const;

        // data can be nullptr which means initial data is undefined and call is basically equivalent to create buffer
        VulkanBuffer CreateBufferWithData(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                                          const void *data, glm::u32 elementSize);

        VulkanBuffer CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

    private:
        const glm::u32 INVALID_MEMORY_TYPE_INDEX = -1; // overflow
        RenderingManager &m_renderingManager;
        VkCommandBuffer m_copyCommandBuffer;
        glm::u32 m_numAllocatedBuffers = 0;
    };
}
