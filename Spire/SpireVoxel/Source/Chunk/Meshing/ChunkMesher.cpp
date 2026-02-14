#include "ChunkMesher.h"

#include "VoxelRenderer.h"
#include "Chunk/Chunk.h"
#include "Edits/BasicVoxelEdit.h"
#include "Utils/ClosestUtil.h"
#include "Utils/ThreadPool.h"

namespace SpireVoxel {
    ChunkMesher::ChunkMesher(VoxelWorld &world,
                             Spire::BufferAllocator &chunkVertexBufferAllocator,
                             Spire::BufferAllocator &chunkVoxelDataBufferAllocator,
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

    struct ToMesh {
        glm::ivec3 ChunkCoords;
        float SquaredDistanceFromCamera;

        bool operator<(const ToMesh &other) const {
            return SquaredDistanceFromCamera < other.SquaredDistanceFromCamera;
        }
    };

    bool ChunkMesher::HandleChunkEdits(std::unordered_set<glm::ivec3> &editedChunks, glm::vec3 cameraCoords) const {
        // find the highest priority chunks
        glm::vec3 cameraChunkCoords = cameraCoords / static_cast<float>(SPIRE_VOXEL_CHUNK_SIZE);
        std::vector<glm::uvec3> chunksToMesh = ClosestUtil::GetClosestCoords(editedChunks, cameraChunkCoords, m_isProfilingMeshing ? UINT32_MAX : m_numCPUThreads);

        std::unordered_map<Chunk *, std::future<ChunkMesh> > meshingChunks;

        // submit mesh tasks to thread pool
        for (glm::uvec3 chunkCoords : chunksToMesh) {
            Chunk *chunk = m_world.TryGetLoadedChunk(chunkCoords);
            if (!chunk) continue;

            meshingChunks[chunk] = Mesh(*chunk);
        }

        // wait for meshing to complete then move meshingChunks into meshedChunks
        if (meshingChunks.empty()) return false;

        std::vector<std::future<void> > meshUploadFutures;
        std::unordered_map<Chunk *, ChunkMesh> meshedChunks;
        meshedChunks.reserve(meshingChunks.size());

        std::shared_ptr<Spire::BufferAllocator::MappedMemory> voxelDataMemory = m_chunkVoxelDataBufferAllocator.MapMemory();
        std::shared_ptr<Spire::BufferAllocator::MappedMemory> vertexBufferMemory = m_chunkVertexBufferAllocator.MapMemory();

        for (auto &[chunk, meshFuture] : meshingChunks) {
            meshedChunks[chunk] = meshFuture.get();
        }
        meshingChunks.clear();

        // Upload meshed chunks to GPU
        for (auto &[chunk, meshFuture] : meshedChunks) {
            editedChunks.erase(chunk->ChunkPosition);
            UploadChunkMesh(*chunk, meshFuture, *voxelDataMemory, *vertexBufferMemory, meshUploadFutures);
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

    void ChunkMesher::UploadChunkMesh(Chunk &chunk, ChunkMesh &mesh, Spire::BufferAllocator::MappedMemory &voxelDataMemory,
                                      Spire::BufferAllocator::MappedMemory &chunkVertexBufferMemory, std::vector<std::future<void> > &futures) const {
        // write the new mesh
        {
            const Spire::BufferAllocator::Allocation oldAllocation = chunk.VertexAllocation;
            chunk.VertexAllocation = {};

            const std::vector<VertexData> &vertexData = mesh.Vertices;

            if (!vertexData.empty()) {
                std::optional alloc = m_chunkVertexBufferAllocator.Allocate(vertexData.size() * sizeof(VertexData));
                if (alloc) {
                    chunk.VertexAllocation = *alloc;

                    // write the mesh into the vertex buffer
                    futures.push_back(
                        Spire::ThreadPool::Instance().submit_task([&chunk,&chunkVertexBufferMemory, &vertexData] {
                            void *memory = chunkVertexBufferMemory.GetByAllocation(chunk.VertexAllocation).Memory;
                            memcpy(static_cast<char *>(memory) + chunk.VertexAllocation.Location.Start,
                                   vertexData.data(), vertexData.size() * sizeof(VertexData));
                        }));
                } else {
                    Spire::error("Chunk vertex data allocation failed");
                }
            }

            if (oldAllocation.Size > 0) {
                m_chunkVertexBufferAllocator.ScheduleFreeAllocation(oldAllocation.Location);
            }

            chunk.NumVertices = vertexData.size();
        }

        // write the voxel data
        {
            const Spire::BufferAllocator::Allocation oldAllocation = chunk.VoxelDataAllocation;
            chunk.VoxelDataAllocation = {};

            if (chunk.NumVertices > 0) {
                std::optional alloc = m_chunkVoxelDataBufferAllocator.Allocate(sizeof(GPUChunkVoxelData));
                if (alloc) {
                    chunk.VoxelDataAllocation = *alloc;

                    // write the voxel data async
                    futures.push_back(Spire::ThreadPool::Instance().submit_task([&voxelDataMemory, &chunk]() {
                        void *memory = voxelDataMemory.GetByAllocation(chunk.VoxelDataAllocation).Memory;
                        memcpy(static_cast<char *>(memory) + chunk.VoxelDataAllocation.Location.Start,
                               chunk.VoxelData.data(), sizeof(GPUChunkVoxelData));
                    }));
                } else {
                    // allocation failed, need to free the vertex allocation
                    Spire::error("Chunk voxel data allocation failed, deallocating vertex buffer");
                    m_chunkVertexBufferAllocator.ScheduleFreeAllocation(chunk.VertexAllocation);
                    chunk.VertexAllocation = {};
                }
            }

            if (oldAllocation.Size > 0) {
                m_chunkVoxelDataBufferAllocator.ScheduleFreeAllocation(oldAllocation.Location);
            }
        }
    }
} // SpireVoxel
