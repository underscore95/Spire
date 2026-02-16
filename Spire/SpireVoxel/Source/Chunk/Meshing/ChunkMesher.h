#pragma once

#include "ChunkMesh.h"
#include "EngineIncludes.h"

namespace SpireVoxel {
    class VoxelWorld;
    struct Chunk;
    struct VertexData;

    class ChunkMesher {
    public:
        ChunkMesher(VoxelWorld &world,
                    Spire::BufferAllocator &chunkVertexBufferAllocator,
                    Spire::BufferAllocator &chunkVoxelDataBufferAllocator,
                    Spire::BufferAllocator &chunkAODataBufferAllocator,
                    bool isProfilingMeshing);

    public:
        // Return true if something was remeshed
        [[nodiscard]] bool HandleChunkEdits(std::unordered_set<glm::ivec3> &editedChunks, glm::vec3 cameraCoords) const;

    private:
        // Mesh a chunk on another thread
        [[nodiscard]] std::future<ChunkMesh> Mesh(Chunk &chunk) const;

        // Upload chunk mesh to GPU
        // voxelDataMemory - mapped memory for m_chunkVoxelDataBufferAllocator
        // chunkVertexBufferMemory - mapped memory for m_chunkVertexBufferAllocator
        // futures - some steps are parallelized, void futures will be pushed to this vector and the function is only complete once all futures return.
        // Until futures haven't returned:
        // chunk, chunkVertexBufferMemory, mesh, and voxelDataMemory must be kept alive
        // the chunks vertex buffer and voxel data allocations must not be changed
        void UploadChunkMesh(Chunk &chunk, ChunkMesh &mesh, Spire::BufferAllocator::MappedMemory &voxelDataMemory, Spire::BufferAllocator::MappedMemory &aoDataMemory,
                             Spire::BufferAllocator::MappedMemory &chunkVertexBufferMemory, std::vector<std::future<void> > &futures) const;

        bool UploadData(Chunk &chunk, Spire::BufferAllocator::MappedMemory &mappedMemory, std::vector<std::future<void>> &futures, glm::u32 requestedSize, const void *data, Spire::BufferAllocator::
                        Allocation &allocation, Spire::BufferAllocator &allocator) const;

    private:
        glm::u32 m_numCPUThreads;
        VoxelWorld &m_world;
        Spire::BufferAllocator &m_chunkVertexBufferAllocator;
        Spire::BufferAllocator &m_chunkVoxelDataBufferAllocator;
        Spire::BufferAllocator &m_chunkAODataBufferAllocator;
        bool m_isProfilingMeshing;
    };
} // SpireVoxel
