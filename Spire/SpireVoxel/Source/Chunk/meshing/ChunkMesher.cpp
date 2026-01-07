#include "ChunkMesher.h"

#include "Chunk/Chunk.h"

namespace SpireVoxel {
    ChunkMesher::ChunkMesher() : m_numCPUThreads(std::thread::hardware_concurrency()) {
        if (m_numCPUThreads == 0) {
            m_numCPUThreads = 8; // Failed to detect number of cores, 8 is a safe assumption
        }
    }

    std::future<ChunkMesh> ChunkMesher::Mesh(Chunk &chunk) const {
        return std::async(std::launch::async, [this, &chunk] {
            return chunk.GenerateMesh();
        });
    }

    glm::u32 ChunkMesher::GetMaxChunksMeshedPerFrame() const {
        return m_numCPUThreads;
    }
} // SpireVoxel
