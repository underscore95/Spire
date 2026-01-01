#include "BufferManager.h"
#include "Utils/Log.h"
#include "Rendering/Core/RenderingCommandManager.h"
#include "Rendering/Core/RenderingDeviceManager.h"
#include "Rendering/RenderingManager.h"
#include "Rendering/Core/Swapchain.h"
#include "Rendering/Core/VulkanQueue.h"
#include "PerImageBuffer.h"
#include "Rendering/Memory/VulkanBuffer.h"
#include "VulkanAllocator.h"

namespace Spire {
    bool s_isDestroyed = false;

    BufferManager::BufferManager(RenderingManager &renderingManager)
        : m_renderingManager(renderingManager) {
        m_renderingManager.GetCommandManager().CreateCommandBuffers(1, &m_copyCommandBuffer);
    }

    BufferManager::~BufferManager() {
        assert(!s_isDestroyed);
        s_isDestroyed = true;

        assert(m_numAllocatedBuffers == 0);

        m_renderingManager.GetCommandManager().FreeCommandBuffers(1, &m_copyCommandBuffer);
    }

    VulkanBuffer BufferManager::CreateIndexBuffer(glm::u32 indexTypeSize, const void *indices, std::size_t numIndices) {
        VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        VkMemoryPropertyFlags memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        return CreateBufferWithData(indexTypeSize * numIndices, usage, memoryProperties, indices, indexTypeSize);
    }

    VulkanBuffer BufferManager::CreateStorageBuffer(const void *elements, std::size_t size, glm::u32 elementSize,
                                                    bool isTransferSource, VkBufferUsageFlags extraUsageFlags) {
        VkBufferUsageFlags usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | extraUsageFlags;
        if (isTransferSource) usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        VkMemoryPropertyFlags memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        return CreateBufferWithData(size, usage, memoryProperties, elements, elementSize);
    }

    VulkanBuffer BufferManager::CreateUniformBuffer(std::size_t size, glm::u32 elementSize, bool isTransferSource) {
        if (isTransferSource) warn("Uniform buffer created as transfer source, is this intended?");

        VkBufferUsageFlags usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        if (isTransferSource) usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        VkMemoryPropertyFlags memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        return CreateBufferWithData(size, usage, memoryProperties, nullptr, elementSize);
    }

    void BufferManager::DestroyBuffer(const VulkanBuffer &buffer) {
        assert(m_numAllocatedBuffers > 0);
        vmaDestroyBuffer(m_renderingManager.GetAllocatorWrapper().GetAllocator(), buffer.Buffer, buffer.Allocation);
        m_numAllocatedBuffers--;
    }

    std::unique_ptr<PerImageBuffer> BufferManager::CreateUniformBuffers(std::size_t bufferSize, std::size_t elementSize, bool isTransferDest) {
        std::vector<VulkanBuffer> buffers;
        buffers.resize(m_renderingManager.GetSwapchain().GetNumImages());

        VkBufferUsageFlags usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        if (isTransferDest) usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        VkMemoryPropertyFlags memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

        for (int i = 0; i < buffers.size(); ++i) {
            buffers[i] = CreateBuffer(bufferSize, usage, memoryProperties, elementSize);
        }

        return std::unique_ptr<PerImageBuffer>(new PerImageBuffer(*this, buffers));
    }

    std::unique_ptr<PerImageBuffer> BufferManager::CreateStorageBuffers(
        std::size_t bufferSize,
        std::size_t numElements,
        const void *data,
        VkBufferUsageFlags extraUsageFlags
    ) {
        std::vector<VulkanBuffer> buffers;
        buffers.resize(m_renderingManager.GetSwapchain().GetNumImages());
        for (int i = 0; i < buffers.size(); ++i) {
            buffers[i] = CreateStorageBuffer(data, bufferSize, bufferSize / numElements, false, extraUsageFlags);
        }

        return std::unique_ptr<PerImageBuffer>(new PerImageBuffer(*this, buffers));
    }

    void BufferManager::UpdateBuffer(const VulkanBuffer &buffer, const void *data, std::size_t size, std::size_t offset) const {
        // TODO use vmaCopyMemoryToAllocation
        assert(size + offset <= buffer.Size);
        void *pMem = nullptr;
        VkResult res = vmaMapMemory(m_renderingManager.GetAllocatorWrapper().GetAllocator(), buffer.Allocation, &pMem);
        if (res != VK_SUCCESS) {
            error("Failed to update buffer memory with size {} bytes ({} bytes starting at {} being read)", buffer.Size, size, offset);
            return;
        }
        memcpy(static_cast<char *>(pMem) + offset, data, size);
        vmaUnmapMemory(m_renderingManager.GetAllocatorWrapper().GetAllocator(), buffer.Allocation);
    }

    void BufferManager::ReadBuffer(const VulkanBuffer &buffer, void *out) const {
        VkResult res = vmaCopyAllocationToMemory(m_renderingManager.GetAllocatorWrapper().GetAllocator(), buffer.Allocation, 0, out, buffer.Size);
        if (res != VK_SUCCESS) {
            error("Failed to update read buffer memory with size {} bytes", buffer.Size);
            return;
        }
    }

    void BufferManager::CmdCopyBuffer(VkCommandBuffer commandBuffer, const VulkanBuffer &source, const VulkanBuffer &dest) {
        assert(source.Size <= dest.Size);
        assert(source.ElementSize == dest.ElementSize);
        VkBufferCopy bufferCopy = {
            .srcOffset = 0,
            .dstOffset = 0,
            .size = source.ElementSize
        };
        vkCmdCopyBuffer(commandBuffer, source.Buffer, dest.Buffer, 1, &bufferCopy);
    }

    bool BufferManager::HasBufferManagerBeenDestroyed() {
        return s_isDestroyed;
    }

    void BufferManager::CopyBuffer(VkBuffer dest, VkBuffer src, VkDeviceSize size) const {
        m_renderingManager.GetCommandManager().BeginCommandBuffer(m_copyCommandBuffer,
                                                                  VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        VkBufferCopy bufferCopy = {
            .srcOffset = 0,
            .dstOffset = 0,
            .size = size
        };

        vkCmdCopyBuffer(m_copyCommandBuffer, src, dest, 1, &bufferCopy);

        vkEndCommandBuffer(m_copyCommandBuffer);

        m_renderingManager.GetQueue().SubmitImmediate(m_copyCommandBuffer);

        m_renderingManager.GetQueue().WaitIdle();
    }

    VulkanBuffer BufferManager::CreateBufferWithData(VkDeviceSize size, VkBufferUsageFlags usage,
                                                     VkMemoryPropertyFlags properties, const void *data,
                                                     glm::u32 elementSize) {
        const std::size_t TEMP_SIZE = 1;
        // create the final buffer
        VulkanBuffer buffer = CreateBuffer(size, usage, properties, TEMP_SIZE); // size overwritten later

        if (data) {
            // create the staging buffer
            VkBufferUsageFlags stagingUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            VkMemoryPropertyFlags stagingProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            VulkanBuffer stagingBuffer = CreateBuffer(size, stagingUsage, stagingProperties, TEMP_SIZE); // buffer deleted so don't care about size

            // copy vertices into staging buffer
            UpdateBuffer(stagingBuffer, data, size);

            // copy the staging buffer to the final buffer
            CopyBuffer(buffer.Buffer, stagingBuffer.Buffer, size);

            // release the resources of the staging buffer
            DestroyBuffer(stagingBuffer);
        }

        // size / element size / count
        assert(elementSize > 0);
        assert(size % elementSize == 0);
        buffer.Count = size / elementSize;
        buffer.Size = size;
        buffer.ElementSize = elementSize;

        return buffer;
    }

    VulkanBuffer BufferManager::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                                             VkMemoryPropertyFlags properties, std::size_t elementSize) {
        assert(size != 0);
        VulkanBuffer buffer;
        buffer.Size = size;
        buffer.ElementSize = elementSize;
        assert(elementSize>0);
        buffer.Count = size / elementSize;
        assert(size % elementSize == 0);

        VkBufferCreateInfo vbCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = size,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE
        };

        VmaAllocationCreateInfo allocInfo = {
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO,
            .requiredFlags = properties
        };

        VkResult res = vmaCreateBuffer(
            m_renderingManager.GetAllocatorWrapper().GetAllocator(),
            &vbCreateInfo,
            &allocInfo,
            &buffer.Buffer,
            &buffer.Allocation,
            nullptr
        );

        if (res != VK_SUCCESS) {
            error("Error creating vulkan buffer of size {}", size);
        } else {
            m_numAllocatedBuffers++;
        }

        assert(buffer.Size != 0);
        return buffer;
    }
}
