#include "ChunkMesher.h"

#include "VoxelRenderer.h"
#include "Chunk/Chunk.h"
#include "Edits/BasicVoxelEdit.h"
#include "Utils/ThreadPool.h"

namespace SpireVoxel {
    ChunkMesher::ChunkMesher(VoxelWorld &world,
                             BufferAllocator &chunkVertexBufferAllocator,
                             BufferAllocator &chunkVoxelDataBufferAllocator,
                             bool isProfilingMeshing)
        : m_numCPUThreads(std::thread::hardware_concurrency()),
          m_world(world),
          m_chunkVertexBufferAllocator(chunkVertexBufferAllocator),
          m_chunkVoxelDataBufferAllocator(chunkVoxelDataBufferAllocator),
          m_isProfilingMeshing(isProfilingMeshing) {
        if (m_numCPUThreads == 0) {
            m_numCPUThreads = 8; // Failed to detect number of cores, 8 is a safe assumption
            Spire::info("Failed to detect number of threads to use, defaulting to {}", m_numCPUThreads);
        } else {
            Spire::info("Using {} threads to mesh chunks", m_numCPUThreads);
        }
    }

    bool ChunkMesher::HandleChunkEdits(std::unordered_set<glm::ivec3> &editedChunks) {
        std::unordered_map<Chunk *, std::future<ChunkMesh> > meshingChunks;

        // submit mesh tasks to thread pool
        for (const auto &chunkPos : editedChunks) {
            Chunk *chunk = m_world.GetLoadedChunk(chunkPos);
            if (!chunk) continue;

            meshingChunks[chunk] = Mesh(*chunk);

            // If profiling, we want to remesh as much as possible
            if (!m_isProfilingMeshing && meshingChunks.size() >= m_numCPUThreads) break;
        }

        // wait for meshing to complete then move meshingChunks into meshedChunks
        if (meshingChunks.empty()) return false;

        std::vector<std::future<void> > meshUploadFutures;
        std::unordered_map<Chunk *, ChunkMesh> meshedChunks;
        meshedChunks.reserve(meshingChunks.size());

        Spire::BufferManager::MappedMemory voxelDataMemory = m_chunkVoxelDataBufferAllocator.MapMemory();
        Spire::BufferManager::MappedMemory vertexBufferMemory = m_chunkVertexBufferAllocator.MapMemory();

        for (auto &[chunk, meshFuture] : meshingChunks) {
            meshedChunks[chunk] = meshFuture.get();
        }
        meshingChunks.clear();

        // Upload meshed chunks to GPU
        for (auto &[chunk, meshFuture] : meshedChunks) {
            editedChunks.erase(chunk->ChunkPosition);
            UploadChunkMesh(*chunk, meshFuture, voxelDataMemory, vertexBufferMemory, meshUploadFutures);
        }

        // Wait for upload tasks to complete
        for (auto &future : meshUploadFutures) {
            future.wait();
        }
        return true;
    }

    std::future<ChunkMesh> ChunkMesher::Mesh(Chunk &chunk) const {
        return Spire::ThreadPool::Instance().submit_task([this, &chunk] {
            return chunk.GenerateMesh();
        });
    }

    void ChunkMesher::UploadChunkMesh(Chunk &chunk, ChunkMesh &mesh, Spire::BufferManager::MappedMemory &voxelDataMemory,
                                      Spire::BufferManager::MappedMemory &chunkVertexBufferMemory, std::vector<std::future<void> > &futures) const {
        // write the new mesh
        {
            const BufferAllocator::Allocation oldAllocation = chunk.VertexAllocation;
            chunk.VertexAllocation = {};

            const std::vector<VertexData> &vertexData = mesh.Vertices;

            if (!vertexData.empty()) {
                std::optional alloc = m_chunkVertexBufferAllocator.Allocate(vertexData.size() * sizeof(VertexData));
                if (alloc) {
                    chunk.VertexAllocation = *alloc;

                    // write the mesh into the vertex buffer
                    futures.push_back(
                        Spire::ThreadPool::Instance().submit_task([&chunk,&chunkVertexBufferMemory, &vertexData] {
                            memcpy(static_cast<char *>(chunkVertexBufferMemory.Memory) + chunk.VertexAllocation.Start, vertexData.data(), vertexData.size() * sizeof(VertexData));
                        }));
                } else {
                    Spire::error("Chunk vertex data allocation failed");
                }
            }

            if (oldAllocation.Size > 0) {
                m_chunkVertexBufferAllocator.ScheduleFreeAllocation(oldAllocation.Start);
            }

            chunk.NumVertices = vertexData.size();
        }

        // write the voxel data
        {
            const BufferAllocator::Allocation oldAllocation = chunk.VoxelDataAllocation;
            chunk.VoxelDataAllocation = {};

            if (chunk.NumVertices > 0) {
                std::optional alloc = m_chunkVoxelDataBufferAllocator.Allocate(sizeof(GPUChunkVoxelData));
                if (alloc) {
                    chunk.VoxelDataAllocation = *alloc;

                    // write the voxel data async
                    futures.push_back(Spire::ThreadPool::Instance().submit_task([&voxelDataMemory, &chunk]() {
                        memcpy(static_cast<char *>(voxelDataMemory.Memory) + chunk.VoxelDataAllocation.Start, chunk.VoxelData.data(), sizeof(GPUChunkVoxelData));
                    }));
                } else {
                    // allocation failed, need to free the vertex allocation
                    Spire::error("Chunk voxel data allocation failed, deallocating vertex buffer");
                    m_chunkVertexBufferAllocator.ScheduleFreeAllocation(chunk.VertexAllocation);
                    chunk.VertexAllocation = {};
                }
            }

            if (oldAllocation.Size > 0) {
                m_chunkVoxelDataBufferAllocator.ScheduleFreeAllocation(oldAllocation.Start);
            }
        }
    }
} // SpireVoxel
