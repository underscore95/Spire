#pragma once

#include "pch.h"
#include "VulkanBuffer.h"

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
        [[nodiscard]] VulkanBuffer CreateIndexBuffer(glm::u32 indexTypeSize, const void *indices, std::size_t numIndices);

        // elements can be nullptr which means that initial data is undefined
        [[nodiscard]] VulkanBuffer CreateStorageBuffer(const void *elements, std::size_t size, glm::u32 elementSize,
                                                       bool isTransferSource = false, VkBufferUsageFlags extraUsageFlags = 0);

        [[nodiscard]] VulkanBuffer CreateUniformBuffer(std::size_t size, glm::u32 elementSize, bool isTransferSource = false);

        void DestroyBuffer(const VulkanBuffer &buffer);

        [[nodiscard]] std::unique_ptr<PerImageBuffer> CreateUniformBuffers(std::size_t bufferSize, std::size_t elementSize, bool isTransferDest = false);

        [[nodiscard]] std::unique_ptr<PerImageBuffer> CreateStorageBuffers(std::size_t bufferSize, std::size_t numElements, const void *data,
                                                                           VkBufferUsageFlags extraUsageFlags = 0);

        void UpdateBuffer(const VulkanBuffer &buffer, const void *data, std::size_t size, std::size_t offset = 0) const;

        // Read elements from buffer into vector, resizes vector if necessary, sync
        template<typename T>
        void ReadBufferElements(const VulkanBuffer &buffer, std::vector<T> &vector) const {
            assert(buffer.ElementSize == sizeof(T));
            // this assert could be removed:
            assert(buffer.Count > 0);
            vector.reserve(buffer.Count);
            ReadBuffer(buffer, vector.data());
        }

        void ReadBuffer(const VulkanBuffer &buffer, void *out) const;

        void CmdCopyBuffer(VkCommandBuffer commandBuffer, const VulkanBuffer& source, const VulkanBuffer & dest);
        void CopyBuffer(VkBuffer dest, VkBuffer src, VkDeviceSize size) const;

        [[nodiscard]] VulkanBuffer CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, std::size_t elementSize);

    public:
        static bool HasBufferManagerBeenDestroyed();

    private:

        // data can be nullptr which means initial data is undefined and call is basically equivalent to create buffer
        [[nodiscard]] VulkanBuffer CreateBufferWithData(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                                                        const void *data, glm::u32 elementSize);

    private:
        const glm::u32 INVALID_MEMORY_TYPE_INDEX = -1; // overflow
        RenderingManager &m_renderingManager;
        VkCommandBuffer m_copyCommandBuffer;
        std::atomic_uint32_t m_numAllocatedBuffers = 0;
    };
}
