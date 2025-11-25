#pragma once
#include <complex.h>
#include <vector>

#include "EngineIncludes.h"

namespace SpireVoxel {
    class BufferAllocator {
    public:
        struct Allocation {
            std::size_t Start = 0;
            std::size_t Size = 0;

            bool operator==(const Allocation &allocation) const { return Start == allocation.Start && Size == allocation.Size; }
        };

        struct PendingFree {
            std::size_t AllocationStart;
            glm::u32 FramesUntilFreed;
        };

        BufferAllocator(Spire::RenderingManager &renderingManager,
                        glm::u32 elementSize,
                        glm::u32 numSwapchainImages,
                        std::size_t bufferSize);

        ~BufferAllocator();

    public:
        std::optional<Allocation> Allocate(std::size_t requestedSize);

        void ScheduleFreeAllocation(std::size_t start);

        void ScheduleFreeAllocation(Allocation allocation);

        Spire::Descriptor GetDescriptor(glm::u32 binding, const std::string &debugName = "Buffer Allocator");

        void Render();

        void Write(Allocation allocation, const void *data, std::size_t size) const;

        // How much memory is either allocated or pending free
        // this is bufferSize - availableMemory
        [[nodiscard]] std::size_t CalculateAllocatedOrPendingMemory() const;

    private:
        [[nodiscard]] std::optional<BufferAllocator::PendingFree> IsPendingFree(std::size_t start) const;

    private:
        Spire::RenderingManager &m_renderingManager;
        Spire::VulkanBuffer m_buffer;
        glm::u32 m_elementSize;
        glm::u32 m_numSwapchainImages;
        std::vector<PendingFree> m_allocationsPendingFree;
        std::map<std::size_t, std::size_t> m_allocations; // start, size
        glm::u32 m_allocationsMade = 0;
        glm::u32 m_pendingFreesMade = 0;
        glm::u32 m_finishedFreesMade = 0;
    };
} // SpireVoxel
