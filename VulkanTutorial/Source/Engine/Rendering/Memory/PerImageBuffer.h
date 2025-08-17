#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "Engine/Utils/MacroDisableCopy.h"

namespace Spire
{
    struct VulkanBuffer;

    class PerImageBuffer
    {
        friend class BufferManager;

    private:
        explicit PerImageBuffer(BufferManager& bufferManager, const std::vector<VulkanBuffer>& buffers);

    public:
        ~PerImageBuffer();

        DISABLE_COPY_AND_MOVE(PerImageBuffer);

    public:
        void Update(glm::u32 imageIndex, const void* data, glm::u32 size, glm::u32 offset = 0) const;

        const VulkanBuffer& GetBuffer(glm::u32 imageIndex) const;
        const std::vector<VulkanBuffer>& GetBuffers() const;

    private:
        std::vector<VulkanBuffer> m_buffers;
        BufferManager& m_bufferManager;
    };
}