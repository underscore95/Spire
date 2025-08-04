#include "BufferManager.h"

#include <spdlog/spdlog.h>

#include "RenderingCommandManager.h"
#include "RenderingDeviceManager.h"
#include "RenderingManager.h"
#include "Swapchain.h"
#include "VulkanQueue.h"

BufferManager::BufferManager(RenderingManager &renderingManager)
    : m_renderingManager(renderingManager) {
    m_renderingManager.GetCommandManager().CreateCommandBuffers(1, &m_copyCommandBuffer);
}

BufferManager::~BufferManager() {
    m_renderingManager.GetCommandManager().FreeCommandBuffers(1, &m_copyCommandBuffer);
}

VulkanBuffer BufferManager::CreateStorageBufferForVertices(const void *pVertices, glm::u32 size) const {
    // Step 1: create the staging buffer
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VkMemoryPropertyFlags memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    VulkanBuffer stagingVb = CreateBuffer(size, usage, memoryProperties);

    // Step 2: map the memory of the stage buffer
    void *pMem = nullptr;
    VkDeviceSize Offset = 0;
    VkMemoryMapFlags Flags = 0;
    VkResult res = vkMapMemory(m_renderingManager.GetDevice(), stagingVb.DeviceMemory, Offset,
                               stagingVb.AllocationSize, Flags, &pMem);
    if (res != VK_SUCCESS) {
        spdlog::error("Failed to map memory for staging buffer");
        return {};
    }

    // Step 3: copy the vertices to the stating buffer
    memcpy(pMem, pVertices, size);

    // Step 4: unmap/release the mapped memory
    vkUnmapMemory(m_renderingManager.GetDevice(), stagingVb.DeviceMemory);

    // Step 5: create the final buffer
    usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VulkanBuffer vertexBuffer = CreateBuffer(size, usage, memoryProperties);

    // Step 6: copy the staging buffer to the final buffer
    CopyBuffer(vertexBuffer.Buffer, stagingVb.Buffer, size);

    // Step 7: release the resources of the staging buffer
    DestroyBuffer(stagingVb);

    return vertexBuffer;
}

void BufferManager::DestroyBuffer(const VulkanBuffer &buffer) const {
    if (buffer.DeviceMemory) {
        vkFreeMemory(m_renderingManager.GetDevice(), buffer.DeviceMemory, nullptr);
    }

    if (buffer.Buffer) {
        vkDestroyBuffer(m_renderingManager.GetDevice(), buffer.Buffer, nullptr);
    }
}

std::vector<VulkanBuffer> BufferManager::CreateUniformBuffers(size_t bufferSize) const {
    std::vector<VulkanBuffer> buffers;
    buffers.resize(m_renderingManager.GetSwapchain().GetNumImages());

    VkBufferUsageFlags usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    VkMemoryPropertyFlags memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    for (int i = 0; i < buffers.size(); ++i) {
        buffers[i] = CreateBuffer(bufferSize, usage, memoryProperties);
    }

    return buffers;
}

void BufferManager::UpdateBuffer(const VulkanBuffer &buffer, const void *data, glm::u32 size, glm::u32 offset) const {
    void *pMem = nullptr;
    VkMemoryMapFlags flags = 0;
    VkResult res = vkMapMemory(m_renderingManager.GetDevice(), buffer.DeviceMemory, offset, size, flags, &pMem);
    if (res != VK_SUCCESS) {
        spdlog::error("Failed to buffer memory with size {} bytes", size);
    }
    memcpy(pMem, data, size);
    vkUnmapMemory(m_renderingManager.GetDevice(), buffer.DeviceMemory);
}

glm::u32 BufferManager::GetMemoryTypeIndex(glm::u32 memTypeBitsMask, VkMemoryPropertyFlags reqMemPropFlags) const {
    const VkPhysicalDeviceMemoryProperties &memoryProperties = m_renderingManager.GetPhysicalDevice().MemoryProperties;

    for (glm::u32 i = 0; i < memoryProperties.memoryTypeCount; i++) {
        const VkMemoryType &memoryType = memoryProperties.memoryTypes[i];
        glm::u32 currentBitmask = (1 << i);

        bool isCurrentMemoryTypeSupported = (memTypeBitsMask & currentBitmask);
        bool hasRequiredMemoryProperties = ((memoryType.propertyFlags & reqMemPropFlags) == reqMemPropFlags);

        if (isCurrentMemoryTypeSupported && hasRequiredMemoryProperties) {
            return i;
        }
    }

    spdlog::error("Cannot find memory type for type {} requested mem props {}", memTypeBitsMask, reqMemPropFlags);
    return INVALID_MEMORY_TYPE_INDEX;
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

    m_renderingManager.GetQueue().SubmitSync(m_copyCommandBuffer);

    m_renderingManager.GetQueue().WaitUntilExecutedAll();
}

VulkanBuffer BufferManager::CreateBuffer(VkDeviceSize Size, VkBufferUsageFlags Usage,
                                         VkMemoryPropertyFlags Properties) const {
    VkBufferCreateInfo vbCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = Size,
        .usage = Usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    VulkanBuffer buffer;

    // Step 1: create a buffer
    VkResult res = vkCreateBuffer(m_renderingManager.GetDevice(), &vbCreateInfo, nullptr, &buffer.Buffer);
    if (res != VK_SUCCESS) {
        spdlog::error("Failed to allocate buffer memory");
    } else {
        spdlog::info("Buffer created");
    }

    // Step 2: get the buffer memory requirements
    VkMemoryRequirements MemReqs = {};
    vkGetBufferMemoryRequirements(m_renderingManager.GetDevice(), buffer.Buffer, &MemReqs);
    spdlog::info("Buffer requires {} bytes", static_cast<int>(MemReqs.size));

    buffer.AllocationSize = MemReqs.size;

    // Step 3: get the memory type index
    glm::u32 memoryTypeIndex = GetMemoryTypeIndex(MemReqs.memoryTypeBits, Properties);
    spdlog::info("Memory type index {}", memoryTypeIndex);
    if (memoryTypeIndex == INVALID_MEMORY_TYPE_INDEX) return {};

    // Step 4: allocate memory
    VkMemoryAllocateInfo memoryAllocateInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = MemReqs.size,
        .memoryTypeIndex = memoryTypeIndex
    };

    res = vkAllocateMemory(m_renderingManager.GetDevice(), &memoryAllocateInfo, nullptr, &buffer.DeviceMemory);
    if (res != VK_SUCCESS) {
        spdlog::error("Failed to allocate buffer memory");
    }

    // Step 5: bind memory
    res = vkBindBufferMemory(m_renderingManager.GetDevice(), buffer.Buffer, buffer.DeviceMemory, 0);
    if (res != VK_SUCCESS) {
        spdlog::error("Failed to bind buffer memory");
    }

    return buffer;
}
