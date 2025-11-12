#pragma once
#include <complex.h>
#include <vector>

#include "EngineIncludes.h"

namespace SpireVoxel {
    class BufferAllocator {
    public:
        struct Allocation {
            glm::u32 Start;
            glm::u32 Size;
        };

        struct PendingFree {
            glm::u32 AllocationStart;
            glm::u32 FramesUntilFreed;
        };

        BufferAllocator(Spire::RenderingManager &renderingManager,
                        glm::u32 elementSize,
                        glm::u32 numSwapchainImages,
                        glm::u32 bufferSize);

        ~BufferAllocator();

    public:
        Allocation Allocate(glm::u32 requestedSize);

        void FreeAllocation(glm::u32 start);

        Spire::Descriptor GetDescriptor(glm::u32 binding, const std::string &debugName = "Buffer Allocator");

        void Update();

        void Write(Allocation allocation, const void *data, glm::u32 size) const;

    private:
        Spire::RenderingManager &m_renderingManager;
        Spire::VulkanBuffer m_buffer;
        glm::u32 m_elementSize;
        glm::u32 m_numSwapchainImages;
        std::vector<PendingFree> m_allocationsPendingFree;
        std::map<glm::u32, glm::u32> m_allocations; // start, size
    };
} // SpireVoxel
