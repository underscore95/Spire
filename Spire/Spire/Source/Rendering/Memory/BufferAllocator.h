#pragma once

#include "BufferManager.h"
#include "pch.h"
#include "Rendering/RenderingManager.h"
#include "Rendering/Descriptors/Descriptor.h"
#include "Utils/Hashing.h"
#include "Utils/MacroDisableCopy.h"

namespace Spire {
    struct VulkanBuffer;
    // Instead of using a million buffers, use a single buffer and allocate sub ranges to different buffers
    // allocations are freed only after inflight frames are no longer using data
    class BufferAllocator {
    public:
        struct AllocationLocation {
            // Index of the buffer is the allocation in
            std::size_t AllocationIndex = SIZE_MAX;
            // Start and size of the allocation in bytes
            std::size_t Start = 0;

            bool operator==(const AllocationLocation &allocation) const {
                return AllocationIndex == allocation.AllocationIndex && Start == allocation.Start;
            }

            friend bool operator<(const AllocationLocation &lhs, const AllocationLocation &rhs) {
                if (lhs.AllocationIndex < rhs.AllocationIndex)
                    return true;
                if (rhs.AllocationIndex < lhs.AllocationIndex)
                    return false;
                return lhs.Start < rhs.Start;
            }
        };

        struct Allocation {
            AllocationLocation Location = {};
            std::size_t Size = 0;

            bool operator==(const Allocation &allocation) const { return Location == allocation.Location && Size == allocation.Size; }
        };

        struct PendingFree {
            AllocationLocation Location;
            glm::u32 FramesUntilFreed;
        };

        // Wrapper for holding mapped memory since internally we use multiple buffers
        // this holds multiple mapped memories
        // in debug mode it asserts the allocator is still in scope before it can be used
        class MappedMemory {
            friend class BufferAllocator;

        private:
            DISABLE_COPY_AND_MOVE(MappedMemory);

            explicit MappedMemory(const BufferAllocator &allocator);

        public:
            ~MappedMemory();

        public:
            [[nodiscard]] Spire::BufferManager::MappedMemory &GetByIndex(std::size_t allocationIndex) const;

            [[nodiscard]] Spire::BufferManager::MappedMemory &GetByAllocation(const Allocation &allocation) const;

        private:
            void PushMappedMemory(const BufferAllocator &allocator, const Spire::VulkanBuffer &buffer);

        private:
            std::vector<std::unique_ptr<Spire::BufferManager::MappedMemory> > m_mappedMemories;
            std::weak_ptr<bool> m_allocatorValid;
        };

    public:
        BufferAllocator(RenderingManager &renderingManager,
                        const std::function<void()> &recreatePipelineCallback,
                        glm::u32 elementSize,
                        glm::u32 numSwapchainImages,
                        std::size_t sizePerInternalBuffer,
                        glm::u32 numInternalBuffers,
                        bool canResize);

        ~BufferAllocator();

    public:
        // Allocate new memory if there is space
        std::optional<Allocation> Allocate(std::size_t requestedSize);

        // Free an allocation once it is no longer needed
        void ScheduleFreeAllocation(AllocationLocation location);

        void ScheduleFreeAllocation(const Allocation &allocation);

        [[nodiscard]] Spire::Descriptor CreateDescriptor(glm::u32 binding, VkShaderStageFlags stages, const std::string &debugName = "Buffer Allocator");

        void Render();

        // Write to allocated memory
        void Write(const Allocation &allocation, const void *data, std::size_t size);

        // Map memory, alternative to Write(), write to yourAllocation.Start
        [[nodiscard]] std::shared_ptr<MappedMemory> MapMemory();

        // How much memory is either allocated or pending free
        // this is bufferSize - availableMemory
        [[nodiscard]] std::size_t CalculateAllocatedOrPendingMemory();

        [[nodiscard]] glm::u32 GetNumElementsPerInternalBuffer() const;

        [[nodiscard]] std::size_t GetTotalSize();

    private:
        [[nodiscard]] std::optional<PendingFree> IsPendingFree(AllocationLocation location);

        void IncreaseCapacity();

        void PushBuffer(glm::u32 elementsInBuffer);

    private:
        Spire::RenderingManager &m_renderingManager;
        std::vector<Spire::VulkanBuffer> m_buffers;
        glm::u32 m_elementSize;
        glm::u32 m_numSwapchainImages;
        std::vector<PendingFree> m_allocationsPendingFree;
        std::map<AllocationLocation, std::size_t> m_allocations; // location, size
        glm::u32 m_allocationsMade = 0;
        glm::u32 m_pendingFreesMade = 0;
        glm::u32 m_finishedFreesMade = 0;
        std::shared_ptr<bool> m_allocatorValid = std::make_shared<bool>(true); // set to false when destroyed
        std::mutex m_mutex;
        std::weak_ptr<MappedMemory> m_mappedMemory;
        std::function<void()> m_recreatePipelineCallback;
        bool m_canResize;
    };
} // SpireVoxel

MAKE_HASHABLE(Spire::BufferAllocator::AllocationLocation, t.AllocationIndex, t.Start)
