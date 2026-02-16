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
                             Spire::BufferAllocator &chunkAODataBufferAllocator,
                             bool isProfilingMeshing)
        : m_numCPUThreads(std::thread::hardware_concurrency()),
          m_world(world),
          m_chunkVertexBufferAllocator(chunkVertexBufferAllocator),
          m_chunkVoxelDataBufferAllocator(chunkVoxelDataBufferAllocator),
          m_chunkAODataBufferAllocator(chunkAODataBufferAllocator),
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
        std::shared_ptr<Spire::BufferAllocator::MappedMemory> aoDataMemory = m_chunkAODataBufferAllocator.MapMemory();

        for (auto &[chunk, meshFuture] : meshingChunks) {
            meshedChunks[chunk] = meshFuture.get();
        }
        meshingChunks.clear();

        // Upload meshed chunks to GPU
        for (auto &[chunk, meshFuture] : meshedChunks) {
            editedChunks.erase(chunk->ChunkPosition);
            UploadChunkMesh(*chunk, meshFuture, *voxelDataMemory, *aoDataMemory, *vertexBufferMemory, meshUploadFutures);
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

    bool ChunkMesher::UploadData(Chunk &chunk, Spire::BufferAllocator::MappedMemory &mappedMemory, std::vector<std::future<void> > &futures, glm::u32 requestedSize,
                                 const void *data, Spire::BufferAllocator::Allocation &allocation, Spire::BufferAllocator &allocator) const {
        const Spire::BufferAllocator::Allocation oldAllocation = allocation;
        allocation = {};

        std::optional<Spire::BufferAllocator::Allocation> alloc;
        if (chunk.NumVertices > 0) {
            alloc = allocator.Allocate(requestedSize);
            if (alloc) {
                allocation = *alloc;

                // write the data async
                futures.push_back(Spire::ThreadPool::Instance().submit_task([&mappedMemory, &allocation, data, alloc]() {
                    void *memory = mappedMemory.GetByAllocation(allocation).Memory;
                    memcpy(static_cast<char *>(memory) + allocation.Location.Start,
                           data, alloc->Size);
                }));
            }
        }

        if (oldAllocation.Size > 0) {
            allocator.ScheduleFreeAllocation(oldAllocation.Location);
        }
        return alloc.has_value();
    }

    void ChunkMesher::UploadChunkMesh(Chunk &chunk, ChunkMesh &mesh, Spire::BufferAllocator::MappedMemory &voxelDataMemory, Spire::BufferAllocator::MappedMemory &aoDataMemory,
                                      Spire::BufferAllocator::MappedMemory &chunkVertexBufferMemory, std::vector<std::future<void> > &futures) const {
        // write the new mesh
        const Spire::BufferAllocator::Allocation oldAllocation = chunk.VertexAllocation;
        chunk.VertexAllocation = {};

        std::vector<VertexData> &vertexData = mesh.Vertices;

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
                vertexData.clear();
            }
        }

        if (oldAllocation.Size > 0) {
            m_chunkVertexBufferAllocator.ScheduleFreeAllocation(oldAllocation.Location);
        }

        chunk.NumVertices = vertexData.size();

        // write the voxel data
        if (!UploadData(chunk, voxelDataMemory, futures, sizeof(mesh.VoxelTypes[0]) * mesh.VoxelTypes.size(), mesh.VoxelTypes.data(), chunk.VoxelDataAllocation,
                        m_chunkVoxelDataBufferAllocator) && chunk.
            NumVertices > 0) {
            // allocation failed, need to free the vertex allocation
            Spire::error("Chunk voxel data allocation failed, deallocating vertex buffer");
            if (chunk.VertexAllocation.Size > 0) m_chunkVertexBufferAllocator.ScheduleFreeAllocation(chunk.VertexAllocation);
            chunk.VertexAllocation = {};
        }

        // write the AO data
        if (!UploadData(chunk, aoDataMemory, futures, sizeof(mesh.AOData[0]) * mesh.AOData.size(), mesh.AOData.data(), chunk.AODataAllocation,
                        m_chunkAODataBufferAllocator) && chunk.NumVertices > 0) {
            // allocation failed, need to free the vertex allocation
            Spire::error("Chunk AO data allocation failed, deallocating other buffers");
            if (chunk.VertexAllocation.Size > 0) m_chunkVertexBufferAllocator.ScheduleFreeAllocation(chunk.VertexAllocation);
            chunk.VertexAllocation = {};

            if (chunk.VoxelDataAllocation.Size > 0) m_chunkVoxelDataBufferAllocator.ScheduleFreeAllocation(chunk.VoxelDataAllocation);
            chunk.VoxelDataAllocation = {};
        }
    }
} // SpireVoxel
