#include "BufferManager.h"
#include <spdlog/spdlog.h>
#include "RenderingCommandManager.h"
#include "RenderingDeviceManager.h"
#include "RenderingManager.h"
#include "Swapchain.h"
#include "VulkanQueue.h"
#include <libassert/assert.hpp>
#include "VulkanBuffer.h"
#include "VulkanAllocator.h"

BufferManager::BufferManager(RenderingManager& renderingManager)
    : m_renderingManager(renderingManager)
{
    m_renderingManager.GetCommandManager().CreateCommandBuffers(1, &m_copyCommandBuffer);
}

BufferManager::~BufferManager()
{
    DEBUG_ASSERT(m_numAllocatedBuffers == 0);

    m_renderingManager.GetCommandManager().FreeCommandBuffers(1, &m_copyCommandBuffer);
}

VulkanBuffer BufferManager::CreateStorageBufferForVertices(const void* vertices, glm::u32 size)
{
    // create the staging buffer
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VkMemoryPropertyFlags memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    VulkanBuffer stagingBuffer = CreateBuffer(size, usage, memoryProperties);

    // copy vertices into staging buffer
    UpdateBuffer(stagingBuffer, vertices, size);

    // create the final buffer
    usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VulkanBuffer vertexBuffer = CreateBuffer(size, usage, memoryProperties);

    // copy the staging buffer to the final buffer
    CopyBuffer(vertexBuffer.Buffer, stagingBuffer.Buffer, size);

    // release the resources of the staging buffer
    DestroyBuffer(stagingBuffer);

    return vertexBuffer;
}

void BufferManager::DestroyBuffer(const VulkanBuffer& buffer)
{
    DEBUG_ASSERT(m_numAllocatedBuffers > 0);
    vmaDestroyBuffer(m_renderingManager.GetAllocatorWrapper().GetAllocator(), buffer.Buffer, buffer.Allocation);
    m_numAllocatedBuffers--;
}

std::vector<VulkanBuffer> BufferManager::CreateUniformBuffers(size_t bufferSize)
{
    std::vector<VulkanBuffer> buffers;
    buffers.resize(m_renderingManager.GetSwapchain().GetNumImages());

    VkBufferUsageFlags usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    VkMemoryPropertyFlags memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    for (int i = 0; i < buffers.size(); ++i)
    {
        buffers[i] = CreateBuffer(bufferSize, usage, memoryProperties);
    }

    return buffers;
}

void BufferManager::UpdateBuffer(const VulkanBuffer& buffer, const void* data, glm::u32 size) const
{
    void* pMem = nullptr;
    VkResult res = vmaMapMemory(m_renderingManager.GetAllocatorWrapper().GetAllocator(), buffer.Allocation, &pMem);
    if (res != VK_SUCCESS)
    {
        spdlog::error("Failed to update buffer memory with size {} bytes", size);
    }
    memcpy(pMem, data, size);
    vmaUnmapMemory(m_renderingManager.GetAllocatorWrapper().GetAllocator(), buffer.Allocation);
}

glm::u32 BufferManager::GetMemoryTypeIndex(glm::u32 memTypeBitsMask, VkMemoryPropertyFlags reqMemPropFlags) const
{
    const VkPhysicalDeviceMemoryProperties& memoryProperties = m_renderingManager.GetPhysicalDevice().MemoryProperties;

    for (glm::u32 i = 0; i < memoryProperties.memoryTypeCount; i++)
    {
        const VkMemoryType& memoryType = memoryProperties.memoryTypes[i];
        glm::u32 currentBitmask = (1 << i);

        bool isCurrentMemoryTypeSupported = (memTypeBitsMask & currentBitmask);
        bool hasRequiredMemoryProperties = ((memoryType.propertyFlags & reqMemPropFlags) == reqMemPropFlags);

        if (isCurrentMemoryTypeSupported && hasRequiredMemoryProperties)
        {
            return i;
        }
    }

    spdlog::error("Cannot find memory type for type {} requested mem props {}", memTypeBitsMask, reqMemPropFlags);
    return INVALID_MEMORY_TYPE_INDEX;
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

    m_renderingManager.GetQueue().SubmitSync(m_copyCommandBuffer);

    m_renderingManager.GetQueue().WaitUntilExecutedAll();
}

VulkanBuffer BufferManager::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
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
