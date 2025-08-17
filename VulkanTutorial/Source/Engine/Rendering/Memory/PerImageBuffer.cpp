#include "PerImageBuffer.h"

#include <libassert/assert.hpp>

#include "BufferManager.h"
#include "VulkanBuffer.h"

namespace Spire
{
    PerImageBuffer::PerImageBuffer(
        BufferManager& bufferManager,
        const std::vector<VulkanBuffer>& buffers)
        : m_buffers(buffers),
          m_bufferManager(bufferManager)
    {
        DEBUG_ASSERT(!BufferManager::HasBufferManagerBeenDestroyed());
    }

    PerImageBuffer::~PerImageBuffer()
    {
        DEBUG_ASSERT(!BufferManager::HasBufferManagerBeenDestroyed());
        for (VulkanBuffer buffer : m_buffers)
        {
            m_bufferManager.DestroyBuffer(buffer);
        }
    }

    void PerImageBuffer::Update(glm::u32 imageIndex, const void* data,  glm::u32 size,glm::u32 offset) const
    {
        m_bufferManager.UpdateBuffer(m_buffers[imageIndex], data, size, offset);
    }

    const VulkanBuffer& PerImageBuffer::GetBuffer(glm::u32 imageIndex) const
    {
        return m_buffers[imageIndex];
    }

    const std::vector<VulkanBuffer>& PerImageBuffer::GetBuffers() const
    {
        return m_buffers;
    }
}