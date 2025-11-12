#include "BufferAllocator.h"

namespace SpireVoxel {
    BufferAllocator::BufferAllocator(
        Spire::RenderingManager &renderingManager,
        glm::u32 elementSize,
        glm::u32 numSwapchainImages,
        glm::u32 bufferSize
    ) : m_renderingManager(renderingManager),
        m_elementSize(elementSize),
        m_numSwapchainImages(numSwapchainImages) {
        m_buffer = m_renderingManager.GetBufferManager().CreateStorageBuffer(nullptr, bufferSize, elementSize);
    }

    BufferAllocator::~BufferAllocator() {
        m_renderingManager.GetBufferManager().DestroyBuffer(m_buffer);
    }

    BufferAllocator::Allocation BufferAllocator::Allocate(glm::u32 requestedSize) {
        glm::u32 previousAllocationEnd = 0;
        for (auto &[start, size] : m_allocations) {
            if (start - previousAllocationEnd >= requestedSize) {
                // there is enough space between these allocations
                m_allocations[previousAllocationEnd] = requestedSize;
                return Allocation{
                    .Start = previousAllocationEnd,
                    .Size = requestedSize
                };
            }

            previousAllocationEnd = start + size;
        }

        // no space previously, need to allocate at end
        assert(previousAllocationEnd + requestedSize <= m_buffer.Size);
        m_allocations[previousAllocationEnd] = requestedSize;
        return Allocation{
            .Start = previousAllocationEnd,
            .Size = requestedSize
        };
    }

    void BufferAllocator::FreeAllocation(glm::u32 start) {
        m_allocationsPendingFree.push_back({start, m_numSwapchainImages + 1});
    }

    Spire::Descriptor BufferAllocator::GetDescriptor(glm::u32 binding, const std::string &debugName) {
        return {
            .ResourceType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .Binding = binding,
            .Stages = VK_SHADER_STAGE_VERTEX_BIT,
            .Resources = {{.Buffer = &m_buffer}},
#ifndef NDEBUG
            .DebugName = debugName
#endif
        };
    }

    void BufferAllocator::Update() {
        for (size_t i = 0; i < m_allocationsPendingFree.size(); i++) {
            if (m_allocationsPendingFree[i].FramesUntilFreed > 0) {
                m_allocationsPendingFree[i].FramesUntilFreed--;
            } else {
                Spire::info("Freeing allocation {}", m_allocationsPendingFree[i].AllocationStart);

                // free it
                m_allocations.erase(m_allocationsPendingFree[i].AllocationStart);
                m_allocationsPendingFree[i] = m_allocationsPendingFree.back();
                m_allocationsPendingFree.pop_back();
            }
        }
    }

    void BufferAllocator::Write(Allocation allocation, const void *data, glm::u32 size) const {
        assert(m_allocations.contains(allocation.Start));
        assert(allocation.Size >= size);
        m_renderingManager.GetBufferManager().UpdateBuffer(m_buffer, data, size, allocation.Start);
    }
} // SpireVoxel
