#include "BufferAllocator.h"

namespace SpireVoxel {
    BufferAllocator::BufferAllocator(
        Spire::RenderingManager &renderingManager,
        glm::u32 elementSize,
        glm::u32 numSwapchainImages,
        std::size_t bufferSize
    ) : m_renderingManager(renderingManager),
        m_elementSize(elementSize),
        m_numSwapchainImages(numSwapchainImages) {
        m_buffer = m_renderingManager.GetBufferManager().CreateStorageBuffer(nullptr, bufferSize, elementSize);
    }

    BufferAllocator::~BufferAllocator() {
        m_renderingManager.GetBufferManager().DestroyBuffer(m_buffer);
    }

    BufferAllocator::Allocation BufferAllocator::Allocate(std::size_t requestedSize) {
        m_allocationsMade++;

        std::size_t previousAllocationEnd = 0;
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
        if (previousAllocationEnd + requestedSize > m_buffer.Size) {
            assert(false); // OUT OF MEMORY :(
            return {};
        }

        m_allocations[previousAllocationEnd] = requestedSize;
        return Allocation{
            .Start = previousAllocationEnd,
            .Size = requestedSize
        };
    }

    void BufferAllocator::ScheduleFreeAllocation(std::size_t start) {
        m_pendingFreesMade++;
        m_allocationsPendingFree.push_back({start, m_numSwapchainImages - 1});
    }

    void BufferAllocator::ScheduleFreeAllocation(Allocation allocation) {
        assert(m_allocations.contains(allocation.Start));
        assert(m_allocations[allocation.Start] == allocation.Size);
        ScheduleFreeAllocation(allocation.Start);
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

    void BufferAllocator::Render() {
        for (size_t i = 0; i < m_allocationsPendingFree.size(); i++) {
            while (i < m_allocationsPendingFree.size() && m_allocationsPendingFree[i].FramesUntilFreed == 0) {
                // free it
                std::size_t numElementsRemoved = m_allocations.erase(m_allocationsPendingFree[i].AllocationStart);
                assert(numElementsRemoved != 0);
                m_allocationsPendingFree[i] = m_allocationsPendingFree.back();
                m_allocationsPendingFree.pop_back();
                m_finishedFreesMade++;
            }

            if (i >= m_allocationsPendingFree.size()) continue;
            m_allocationsPendingFree[i].FramesUntilFreed--;
        }
    }

    void BufferAllocator::Write(Allocation allocation, const void *data, std::size_t size) const {
        assert(m_allocations.contains(allocation.Start));
        assert(allocation.Size >= size);
        m_renderingManager.GetBufferManager().UpdateBuffer(m_buffer, data, size, allocation.Start);
    }

    glm::u64 BufferAllocator::CalculateAllocatedOrPendingMemory() const {
        constexpr bool LOG_ALLOCATIONS = false;

        glm::u64 allocated = 0;
        for (auto &alloc : m_allocations) {
            allocated += alloc.second;
        }

        if constexpr (LOG_ALLOCATIONS) {
            // ReSharper disable once CppDFAUnreachableCode
            Spire::info("Num allocations: {} (expected {})", m_allocations.size(), m_allocationsMade - m_finishedFreesMade);
            Spire::info("Num allocations pending free: {}", m_allocationsPendingFree.size());
            for (auto &alloc : m_allocations) {
                std::optional pendingFree = IsPendingFree(alloc.first);
                Spire::info(
                    "allocation start: {} - size: {} - pending free: {}",
                    alloc.first,
                    alloc.second,
                    pendingFree.has_value() ? std::format("freed in {} frames", pendingFree->FramesUntilFreed) : "no"
                );
            }
        }

        return allocated;
    }

    std::optional<BufferAllocator::PendingFree> BufferAllocator::IsPendingFree(std::size_t start) const {
        for (auto &pending : m_allocationsPendingFree) {
            if (pending.AllocationStart == start) {
                return {pending};
            }
        }
        return {};
    }
} // SpireVoxel
