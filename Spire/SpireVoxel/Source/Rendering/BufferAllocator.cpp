#include "BufferAllocator.h"

namespace SpireVoxel {
    BufferAllocator::MappedMemory::MappedMemory(const BufferAllocator &allocator)
        : m_allocatorValid(allocator.m_allocatorValid) {
        m_mappedMemories.reserve(allocator.m_buffers.size());
        for (auto &buffer : allocator.m_buffers) {
            m_mappedMemories.push_back(std::move(allocator.m_renderingManager.GetBufferManager().Map(buffer)));
        }
    }

    BufferAllocator::MappedMemory::~MappedMemory() {
        auto valid = m_allocatorValid.lock();
        assert(valid);
        assert(*valid);
    }

    Spire::BufferManager::MappedMemory &BufferAllocator::MappedMemory::GetByIndex(std::size_t allocationIndex) const {
        assert(allocationIndex != SIZE_MAX);
        auto valid = m_allocatorValid.lock();
        assert(valid);
        assert(*valid);
        return *m_mappedMemories[allocationIndex];
    }

    Spire::BufferManager::MappedMemory &BufferAllocator::MappedMemory::GetByAllocation(const Allocation &allocation) const {
        return GetByIndex(allocation.Location.AllocationIndex);
    }

    BufferAllocator::BufferAllocator(
        Spire::RenderingManager &renderingManager,
        glm::u32 elementSize,
        glm::u32 numSwapchainImages,
        std::size_t allocatorSize
    ) : m_renderingManager(renderingManager),
        m_elementSize(elementSize),
        m_numSwapchainImages(numSwapchainImages) {
        VkDeviceSize maxBufferSize = m_renderingManager.GetPhysicalDevice().DeviceProperties.limits.maxStorageBufferRange;

        static bool hasLoggedMaxSSBOSize = false;
        if (!hasLoggedMaxSSBOSize) {
            Spire::info("Maximum SSBO Size: {} MB", maxBufferSize / 1024 / 1024);
            hasLoggedMaxSSBOSize = true;
        }

        std::size_t maxElementsPerBuffer = maxBufferSize / elementSize;
        std::size_t numElements = allocatorSize / elementSize;
        std::size_t buffersNeeded = glm::ceil(static_cast<double>(numElements) / static_cast<double>(maxElementsPerBuffer));
        std::size_t numElementsInLastBuffer = numElements % maxElementsPerBuffer;

        for (std::size_t i = 0; i < buffersNeeded - 1; i++) {
            m_buffers.push_back(m_renderingManager.GetBufferManager().CreateStorageBuffer(nullptr, maxBufferSize, elementSize));
        }
        m_buffers.push_back(m_renderingManager.GetBufferManager().CreateStorageBuffer(nullptr, numElementsInLastBuffer * elementSize, elementSize));

        // Test
        std::size_t totalSize = 0;
        for (const auto &buffer : m_buffers) {
            totalSize += buffer.Size;
            assert(buffer.Size % elementSize == 0);
        }
        assert(totalSize == allocatorSize);
        assert(m_buffers.size() == 1);
    }

    BufferAllocator::~BufferAllocator() {
        *m_allocatorValid = false;
        for (const auto &buffer : m_buffers) {
            m_renderingManager.GetBufferManager().DestroyBuffer(buffer);
        }
    }

    std::optional<BufferAllocator::Allocation> BufferAllocator::Allocate(std::size_t requestedSize) {
        m_allocationsMade++;

        for (std::size_t allocationIndex = 0; allocationIndex < m_buffers.size(); allocationIndex++) {
            std::size_t previousAllocationEnd = 0;
            for (auto &[location, size] : m_allocations) {
                if (location.AllocationIndex != allocationIndex) continue;
                if (location.Start - previousAllocationEnd >= requestedSize) {
                    // there is enough space between these allocations
                    AllocationLocation newLocation = {allocationIndex, previousAllocationEnd};
                    m_allocations[newLocation] = requestedSize;
                    return Allocation{
                        .Location = newLocation,
                        .Size = requestedSize
                    };
                }

                previousAllocationEnd = location.Start + size;
            }

            // no space previously, need to allocate at end of this buffer!

            // does this buffer have space?
            if (previousAllocationEnd + requestedSize > m_buffers[allocationIndex].Size) {
                continue;
            }

            AllocationLocation newLocation = {allocationIndex, previousAllocationEnd};
            m_allocations[newLocation] = requestedSize;
            return Allocation{
                .Location = newLocation,
                .Size = requestedSize
            };
        }

        Spire::warn("Failed to allocate memory in BufferAllocator - out of memory! Requested allocation size: {}", requestedSize);
        return std::nullopt;
    }

    void BufferAllocator::ScheduleFreeAllocation(AllocationLocation location) {
        assert(m_allocations.contains(location));
        m_pendingFreesMade++;
        m_allocationsPendingFree.push_back({location, m_numSwapchainImages - 1});
    }

    void BufferAllocator::ScheduleFreeAllocation(Allocation allocation) {
        assert(m_allocations.contains(allocation.Location));
        assert(m_allocations[allocation.Location] == allocation.Size);
        ScheduleFreeAllocation(allocation.Location);
    }

    Spire::Descriptor BufferAllocator::CreateDescriptor(glm::u32 binding, VkShaderStageFlags stages, const std::string &debugName) const {
        Spire::Descriptor descriptor = {
            .ResourceType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .Binding = binding,
            .Stages = stages,
            .Resources = std::vector<Spire::Descriptor::ResourcePtr>(m_buffers.size()),
#ifndef NDEBUG
            .DebugName = debugName
#endif
        };

        for (std::size_t i = 0; i < m_buffers.size(); i++) {
            descriptor.Resources[i].Buffer = &(m_buffers[i]);
        }

        return descriptor;
    }

    void BufferAllocator::Render() {
        for (size_t i = 0; i < m_allocationsPendingFree.size(); i++) {
            while (i < m_allocationsPendingFree.size() && m_allocationsPendingFree[i].FramesUntilFreed == 0) {
                // free it
                std::size_t numElementsRemoved = m_allocations.erase(m_allocationsPendingFree[i].Location);
                assert(numElementsRemoved != 0);
                m_allocationsPendingFree[i] = m_allocationsPendingFree.back();
                m_allocationsPendingFree.pop_back();
                m_finishedFreesMade++;

                // we don't actually write anything here, it is just marked as memory that can be allocated and the data will be overwritten
            }

            if (i >= m_allocationsPendingFree.size()) continue;
            m_allocationsPendingFree[i].FramesUntilFreed--;
        }
    }

    void BufferAllocator::Write(const Allocation &allocation, const void *data, std::size_t size) const {
        assert(m_allocations.contains(allocation.Location));
        assert(allocation.Size >= size);
        m_renderingManager.GetBufferManager().UpdateBuffer(m_buffers[allocation.Location.AllocationIndex], data, size, allocation.Location.Start);
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

    std::unique_ptr<BufferAllocator::MappedMemory> BufferAllocator::MapMemory() const {
        return std::unique_ptr<MappedMemory>(new MappedMemory(*this));
    }

    std::optional<BufferAllocator::PendingFree> BufferAllocator::IsPendingFree(AllocationLocation location) const {
        for (auto &pending : m_allocationsPendingFree) {
            if (pending.Location == location) {
                return {pending};
            }
        }
        return {};
    }
} // SpireVoxel
