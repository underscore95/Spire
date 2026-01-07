#pragma once
#include "pch.h"
#include "VulkanBuffer.h"
#include "Utils/MacroDisableCopy.h"

namespace Spire {
    class PerImageBuffer;
    struct VulkanBuffer;
    class RenderingManager;

    class BufferManager {
        friend class ImageManager;

    public:
        class MappedMemory {
            friend class BufferManager;

        private:
            DISABLE_COPY_AND_MOVE(MappedMemory);
            explicit MappedMemory(RenderingManager &rm, const VulkanBuffer &buffer);

        public:
            ~MappedMemory();

        public:
            void *Memory; // Pointer to the mapped memory, you can write but not resize
            const std::size_t Size; // Size of mapped memory

        private:
            VulkanBuffer m_buffer;
            RenderingManager &m_renderingManager;
        };

    public:
        explicit BufferManager(RenderingManager &renderingManager);

        ~BufferManager();

    public:
        // Create an index buffer, indexTypeSize determines type of indices, either sizeof(std::uint32_t) or sizeof(std::uint16_t) or sizeof(std::uint8_t)
        [[nodiscard]] VulkanBuffer CreateIndexBuffer(glm::u32 indexTypeSize, const void *indices, std::size_t numIndices);

        // elements can be nullptr which means that initial data is undefined
        [[nodiscard]] VulkanBuffer CreateStorageBuffer(const void *elements, std::size_t size, glm::u32 elementSize,
                                                       bool isTransferSource = false, VkBufferUsageFlags extraUsageFlags = 0);

        [[nodiscard]] VulkanBuffer CreateUniformBuffer(std::size_t size, glm::u32 elementSize, bool isTransferSource = false);

        void DestroyBuffer(const VulkanBuffer &buffer);

        [[nodiscard]] std::unique_ptr<PerImageBuffer> CreateUniformBuffers(std::size_t bufferSize, bool isTransferDest = false);

        [[nodiscard]] std::unique_ptr<PerImageBuffer> CreateStorageBuffers(std::size_t bufferSize, std::size_t numElements, const void *data,
                                                                           VkBufferUsageFlags extraUsageFlags = 0);

        void UpdateBuffer(const VulkanBuffer &buffer, const void *data, std::size_t size, std::size_t offset = 0) const;

        // Map memory, automatically unmap memory on destroy (uploads to gpu)
        [[nodiscard]] MappedMemory Map(const VulkanBuffer &buffer) const;

    public:
        static bool HasBufferManagerBeenDestroyed();

    private:
        void CopyBuffer(VkBuffer dest, VkBuffer src, VkDeviceSize size) const;

        // data can be nullptr which means initial data is undefined and call is basically equivalent to create buffer
        [[nodiscard]] VulkanBuffer CreateBufferWithData(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                                                        const void *data, glm::u32 elementSize);

        [[nodiscard]] VulkanBuffer CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

    private:
        const glm::u32 INVALID_MEMORY_TYPE_INDEX = -1; // overflow
        RenderingManager &m_renderingManager;
        VkCommandBuffer m_copyCommandBuffer;
        glm::u32 m_numAllocatedBuffers = 0;
    };
}
