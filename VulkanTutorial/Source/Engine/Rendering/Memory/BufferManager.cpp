#include "BufferManager.h"
#include <spdlog/spdlog.h>
#include "Engine/Rendering/Core/RenderingCommandManager.h"
#include "Engine/Rendering/Core/RenderingDeviceManager.h"
#include "Engine/Rendering/RenderingManager.h"
#include "Engine/Rendering/Core/Swapchain.h"
#include "Engine/Rendering/Core/VulkanQueue.h"
#include <libassert/assert.hpp>

#include "PerImageBuffer.h"
#include "Engine/Rendering/Memory/VulkanBuffer.h"
#include "VulkanAllocator.h"

namespace Spire
{
    bool s_isDestroyed = false;

    BufferManager::BufferManager(RenderingManager& renderingManager)
        : m_renderingManager(renderingManager)
    {
        m_renderingManager.GetCommandManager().CreateCommandBuffers(1, &m_copyCommandBuffer);
    }

    BufferManager::~BufferManager()
    {
        DEBUG_ASSERT(!s_isDestroyed);
        s_isDestroyed = true;

        DEBUG_ASSERT(m_numAllocatedBuffers == 0);

        m_renderingManager.GetCommandManager().FreeCommandBuffers(1, &m_copyCommandBuffer);
    }

    VulkanBuffer BufferManager::CreateIndexBuffer(glm::u32 indexTypeSize, const void* indices, glm::u32 numIndices)
    {
        VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        VkMemoryPropertyFlags memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        return CreateBufferWithData(indexTypeSize * numIndices, usage, memoryProperties, indices, indexTypeSize);
    }

    VulkanBuffer BufferManager::CreateStorageBuffer(const void* elements, glm::u32 size, glm::u32 elementSize,
                                                    bool isTransferSource)
    {
        VkBufferUsageFlags usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        if (isTransferSource) usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        VkMemoryPropertyFlags memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        return CreateBufferWithData(size, usage, memoryProperties, elements, elementSize);
    }

    void BufferManager::DestroyBuffer(const VulkanBuffer& buffer)
    {
        DEBUG_ASSERT(m_numAllocatedBuffers > 0);
        vmaDestroyBuffer(m_renderingManager.GetAllocatorWrapper().GetAllocator(), buffer.Buffer, buffer.Allocation);
        m_numAllocatedBuffers--;
    }

    std::unique_ptr<PerImageBuffer> BufferManager::CreateUniformBuffers(size_t bufferSize, bool isTransferDest)
    {
        std::vector<VulkanBuffer> buffers;
        buffers.resize(m_renderingManager.GetSwapchain().GetNumImages());

        VkBufferUsageFlags usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        if (isTransferDest) usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        VkMemoryPropertyFlags memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

        for (int i = 0; i < buffers.size(); ++i)
        {
            buffers[i] = CreateBuffer(bufferSize, usage, memoryProperties);
        }

        return std::unique_ptr<PerImageBuffer>(new PerImageBuffer(*this, buffers));
    }

    std::unique_ptr<PerImageBuffer> BufferManager::CreateStorageBuffers(
        size_t bufferSize,
        size_t numElements,
        const void* data
    )
    {
        std::vector<VulkanBuffer> buffers;
        buffers.resize(m_renderingManager.GetSwapchain().GetNumImages());
        for (int i = 0; i < buffers.size(); ++i)
        {
            buffers[i] = CreateStorageBuffer(data, bufferSize, bufferSize / numElements);
        }

        return std::unique_ptr<PerImageBuffer>(new PerImageBuffer(*this, buffers));
    }

    void BufferManager::UpdateBuffer(const VulkanBuffer& buffer, const void* data, glm::u32 size, glm::u32 offset) const
    {
        void* pMem = nullptr;
        VkResult res = vmaMapMemory(m_renderingManager.GetAllocatorWrapper().GetAllocator(), buffer.Allocation, &pMem);
        if (res != VK_SUCCESS)
        {
            spdlog::error("Failed to update buffer memory with size {} bytes", size);
            return;
        }
        memcpy(static_cast<char*>(pMem) + offset, data, size);
        vmaUnmapMemory(m_renderingManager.GetAllocatorWrapper().GetAllocator(), buffer.Allocation);
    }

    bool BufferManager::HasBufferManagerBeenDestroyed()
    {
        return s_isDestroyed;
    }

    void BufferManager::CopyBuffer(VkBuffer dest, VkBuffer src, VkDeviceSize size) const
    {
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
                                                     VkMemoryPropertyFlags properties, const void* data,
                                                     glm::u32 elementSize)
    {
        // create the final buffer
        VulkanBuffer buffer = CreateBuffer(size, usage, properties);

        if (data)
        {
            // create the staging buffer
            VkBufferUsageFlags stagingUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            VkMemoryPropertyFlags stagingProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            VulkanBuffer stagingBuffer = CreateBuffer(size, stagingUsage, stagingProperties);

            // copy vertices into staging buffer
            UpdateBuffer(stagingBuffer, data, size);

            // copy the staging buffer to the final buffer
            CopyBuffer(buffer.Buffer, stagingBuffer.Buffer, size);

            // release the resources of the staging buffer
            DestroyBuffer(stagingBuffer);
        }

        // size / element size / count
        DEBUG_ASSERT(elementSize > 0);
        DEBUG_ASSERT(size % elementSize == 0);
        buffer.Count = size / elementSize;
        buffer.Size = size;
        buffer.ElementSize = elementSize;

        return buffer;
    }

    VulkanBuffer BufferManager::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                                             VkMemoryPropertyFlags properties)
    {
        VulkanBuffer buffer;

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

        if (res != VK_SUCCESS)
        {
            spdlog::error("Error creating vulkan buffer of size {}", size);
        }
        else
        {
            m_numAllocatedBuffers++;
        }

        return buffer;
    }
}
