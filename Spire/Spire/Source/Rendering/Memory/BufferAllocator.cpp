#include "BufferAllocator.h"

#include "BufferManager.h"
#include "Rendering/Core/RenderingDeviceManager.h"
#include "Utils/Log.h"

namespace Spire {
    BufferAllocator::MappedMemory::MappedMemory(const BufferAllocator &allocator)
        : m_allocatorValid(allocator.m_allocatorValid) {
        m_mappedMemories.reserve(allocator.m_buffers.size());
        for (auto &buffer : allocator.m_buffers) {
            PushMappedMemory(allocator, buffer);
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

    void BufferAllocator::MappedMemory::PushMappedMemory(const BufferAllocator &allocator, const Spire::VulkanBuffer &buffer) {
        m_mappedMemories.push_back(std::move(allocator.m_renderingManager.GetBufferManager().Map(buffer)));
    }

    BufferAllocator::BufferAllocator(
     Spire::RenderingManager &renderingManager,
     const std::function<void()> &recreatePipelineCallback,
     glm::u32 elementSize,
     glm::u32 numSwapchainImages,
     std::size_t sizePerInternalBuffer,
     glm::u32 numInternalBuffers,
     bool canResize
 ) : m_renderingManager(renderingManager),
     m_elementSize(elementSize),
     m_numSwapchainImages(numSwapchainImages),
     m_recreatePipelineCallback(recreatePipelineCallback),
    m_canResize(canResize){

        // Get max buffer size
        VkDeviceSize maxBufferSize =
            m_renderingManager.GetPhysicalDevice().DeviceProperties.limits.maxStorageBufferRange;

        static bool hasLoggedMaxSSBOSize = false;
        if (!hasLoggedMaxSSBOSize) {
            Spire::info("Maximum SSBO Size: {} MB", maxBufferSize / 1024 / 1024);
            hasLoggedMaxSSBOSize = true;
        }

        // Check if the requested size is less than max size
        assert(sizePerInternalBuffer % elementSize == 0);

        std::size_t requestedElementsPerBuffer = sizePerInternalBuffer / elementSize;
        std::size_t maxElementsPerBuffer = maxBufferSize / elementSize;

        if (requestedElementsPerBuffer > maxElementsPerBuffer) {
            Spire::warn(
                "Requested {} elements per internal buffer, clamping to device limit {}",
                requestedElementsPerBuffer,
                maxElementsPerBuffer
            );
            requestedElementsPerBuffer = maxElementsPerBuffer;
        }

        assert(requestedElementsPerBuffer > 0);
        assert(requestedElementsPerBuffer <= UINT32_MAX);

        // Create buffers
        for (glm::u32 i = 0; i < numInternalBuffers; ++i) {
            PushBuffer(static_cast<glm::u32>(requestedElementsPerBuffer));
        }

        // Test
        for (const auto &buffer : m_buffers) {
            assert(buffer.Size % elementSize == 0);
            assert(buffer.Size <= maxBufferSize);
        }
    }

    BufferAllocator::~BufferAllocator() {
        std::unique_lock lock(m_mutex);
        *m_allocatorValid = false;
        for (const auto &buffer : m_buffers) {
            m_renderingManager.GetBufferManager().DestroyBuffer(buffer);
        }
    }

    std::optional<BufferAllocator::Allocation> BufferAllocator::Allocate(std::size_t requestedSize) {
        std::unique_lock lock(m_mutex);
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

        if (m_canResize) {
            Spire::info("Allocating memory in BufferAllocator caused capacity to be increased (actual GPU allocation)! Requested allocation size: {}", requestedSize);
            IncreaseCapacity();
        } else {
            return std::nullopt;
        }

        lock.unlock();
        auto alloc = Allocate(requestedSize);
        return alloc;
    }

    void BufferAllocator::ScheduleFreeAllocation(AllocationLocation location) {
        std::unique_lock lock(m_mutex);
        assert(m_allocations.contains(location));
        m_pendingFreesMade++;
        m_allocationsPendingFree.push_back({location, m_numSwapchainImages - 1});
    }

    void BufferAllocator::ScheduleFreeAllocation(Allocation allocation) {
        std::unique_lock lock(m_mutex);
        assert(m_allocations.contains(allocation.Location));
        assert(m_allocations[allocation.Location] == allocation.Size);
        ScheduleFreeAllocation(allocation.Location);
    }

    Spire::Descriptor BufferAllocator::CreateDescriptor(glm::u32 binding, VkShaderStageFlags stages, const std::string &debugName) {
        Descriptor descriptor = {
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
        std::unique_lock lock(m_mutex);
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

    void BufferAllocator::Write(const Allocation &allocation, const void *data, std::size_t size) {
        std::unique_lock lock(m_mutex);
        assert(m_allocations.contains(allocation.Location));
        assert(allocation.Size >= size);
        m_renderingManager.GetBufferManager().UpdateBuffer(m_buffers[allocation.Location.AllocationIndex], data, size, allocation.Location.Start);
    }

    glm::u64 BufferAllocator::CalculateAllocatedOrPendingMemory() {
        std::unique_lock lock(m_mutex);
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

    glm::u32 BufferAllocator::GetNumElementsPerInternalBuffer() const {
        return m_buffers[0].Count;
    }

    std::size_t BufferAllocator::GetTotalSize() {
        std::unique_lock lock(m_mutex);
        return m_buffers[0].Size * m_buffers.size();
    }

    std::shared_ptr<BufferAllocator::MappedMemory> BufferAllocator::MapMemory() {
        std::unique_lock lock(m_mutex);
        assert(!m_mappedMemory.lock());
        auto mappedMemory = std::shared_ptr<MappedMemory>(new MappedMemory(*this));
        m_mappedMemory = mappedMemory;
        return mappedMemory;
    }

    std::optional<BufferAllocator::PendingFree> BufferAllocator::IsPendingFree(AllocationLocation location) {
        std::unique_lock lock(m_mutex);
        for (auto &pending : m_allocationsPendingFree) {
            if (pending.Location == location) {
                return {pending};
            }
        }
        return {};
    }

    // Note this does not lock the mutex
    void BufferAllocator::IncreaseCapacity() {
        assert(!m_buffers.empty());
        PushBuffer(m_buffers[0].Count);

        std::shared_ptr mappedMemory = m_mappedMemory.lock();
        if (mappedMemory) {
            mappedMemory->PushMappedMemory(*this, m_buffers.back());
        }

        m_recreatePipelineCallback();
    }

    void BufferAllocator::PushBuffer(glm::u32 elementsInBuffer) {
        m_buffers.push_back(m_renderingManager.GetBufferManager().CreateStorageBuffer(nullptr, elementsInBuffer * m_elementSize, m_elementSize));
    }
} // Spire
