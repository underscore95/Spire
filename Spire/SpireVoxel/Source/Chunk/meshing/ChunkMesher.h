#pragma once

#include "ChunkMesh.h"
#include "EngineIncludes.h"

namespace SpireVoxel {
    struct Chunk;
}

namespace SpireVoxel {
    struct VertexData;

    class ChunkMesher {
    public:
        ChunkMesher();

    public:
        // Mesh a chunk on another thread
        [[nodiscard]] std::future<ChunkMesh> Mesh(Chunk &chunk) const;

        // Equal to the number of threads in the CPU
        [[nodiscard]] glm::u32 GetMaxChunksMeshedPerFrame() const;

    private:
        glm::u32 m_numCPUThreads;
    };
} // SpireVoxel
