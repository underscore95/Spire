#pragma once

#include "ChunkMesh.h"
#include "EngineIncludes.h"

namespace SpireVoxel {
    class VoxelWorld;
    class BufferAllocator;
    struct Chunk;
    struct VertexData;

    class ChunkMesher {
    public:
        ChunkMesher(VoxelWorld &world,
                    BufferAllocator &chunkVertexBufferAllocator,
                    BufferAllocator &chunkVoxelDataBufferAllocator,
                    bool isProfilingMeshing);

    public:
        // Return true if something was remeshed
        [[nodiscard]] bool HandleChunkEdits(std::unordered_set<glm::ivec3> &editedChunks);

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
        void UploadChunkMesh(Chunk &chunk, ChunkMesh &mesh, Spire::BufferManager::MappedMemory &voxelDataMemory, Spire::BufferManager::MappedMemory &chunkVertexBufferMemory,
                             std::vector<std
                                 ::future<void> > &futures) const;

    private:
        glm::u32 m_numCPUThreads;
        VoxelWorld &m_world;
        BufferAllocator &m_chunkVertexBufferAllocator;
        BufferAllocator &m_chunkVoxelDataBufferAllocator;
        bool m_isProfilingMeshing;
    };
} // SpireVoxel
