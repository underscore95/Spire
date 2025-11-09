#include "Chunk.h"

#include "VoxelWorld.h"

namespace SpireVoxel {
    Chunk::Chunk(Spire::RenderingManager &renderingManager,
                 glm::ivec3 chunkPosition,
                 VoxelWorld &world)
        : m_renderingManager(renderingManager),
          m_chunkPosition(chunkPosition),
          m_world(world) {
        OnChunkEdited();
    }

    Chunk::~Chunk() {
        FreeBuffersIfAllocated();
    }

    Chunk::Chunk(Chunk &&other)
        noexcept : m_renderingManager(other.m_renderingManager),
                   m_chunkPosition(other.m_chunkPosition),
                   m_world(other.m_world) {
        m_vertexStorageBuffer = other.m_vertexStorageBuffer;
        m_voxelData = other.m_voxelData;

        other.m_vertexStorageBuffer = {};
    }

    Chunk &Chunk::operator=(Chunk &&other) noexcept {
        FreeBuffersIfAllocated();

        assert(& m_renderingManager == &other.m_renderingManager);
        m_chunkPosition = other.m_chunkPosition;
        m_vertexStorageBuffer = other.m_vertexStorageBuffer;
        m_voxelData = other.m_voxelData;

        other.m_vertexStorageBuffer = {};

        return *this;
    }

    void Chunk::SetVoxel(glm::vec3 pos, std::int32_t type) {
        m_voxelData[SPIRE_VOXEL_POSITION_TO_INDEX(pos)] = type;
        OnChunkEdited();
    }

    void Chunk::SetVoxelRect(glm::vec3 pos, glm::vec3 rectDimensions, std::int32_t type) {
        for (size_t x = 0; x < rectDimensions.x; ++x) {
            for (size_t y = 0; y < rectDimensions.y; ++y) {
                for (size_t z = 0; z < rectDimensions.z; ++z) {
                    // todo: optimise, surely we don't need a memory write for every single voxel?
                    m_voxelData[SPIRE_VOXEL_POSITION_XYZ_TO_INDEX(pos.x + x, pos.y + y, pos.z + z)] = type;
                }
            }
        }

        OnChunkEdited();
    }

    const Spire::VulkanBuffer &Chunk::GetVertexBuffer() const {
        return m_vertexStorageBuffer;
    }

    bool Chunk::HasMesh() const {
        return m_vertexStorageBuffer.Buffer != VK_NULL_HANDLE;
    }

    ChunkData Chunk::GetChunkData() const {
        return {m_vertexStorageBuffer.Count};
    }

    void Chunk::OnChunkEdited() {
        glm::u32 previousNumVertices = m_vertexStorageBuffer.Count;
        RegenerateMesh(); // todo if it was just changing type and we can reuse mesh, don't regenerate it
        m_world.OnChunkEdited(*this, previousNumVertices);
    }

    void Chunk::RegenerateMesh() {
        // generate vertices & indices
        std::vector<VertexData> vertices;

        for (size_t i = 0; i < m_voxelData.size(); i++) {
            if (m_voxelData[i] == 0) continue;

            glm::vec3 p = SPIRE_VOXEL_INDEX_TO_POSITION(glm::vec3, i) + glm::vec3(m_chunkPosition * SPIRE_VOXEL_CHUNK_SIZE);

            // Front (Z+)
            vertices.push_back({m_voxelData[i], p.x + 0, p.y + 0, p.z + 1, 0, 0});
            vertices.push_back({m_voxelData[i], p.x + 1, p.y + 0, p.z + 1, 1, 0});
            vertices.push_back({m_voxelData[i], p.x + 1, p.y + 1, p.z + 1, 1, 1});
            vertices.push_back({m_voxelData[i], p.x + 1, p.y + 1, p.z + 1, 1, 1});
            vertices.push_back({m_voxelData[i], p.x + 0, p.y + 1, p.z + 1, 0, 1});
            vertices.push_back({m_voxelData[i], p.x + 0, p.y + 0, p.z + 1, 0, 0});

            // Back (Z-)
            vertices.push_back({m_voxelData[i], p.x + 1, p.y + 0, p.z + 0, 0, 0});
            vertices.push_back({m_voxelData[i], p.x + 0, p.y + 0, p.z + 0, 1, 0});
            vertices.push_back({m_voxelData[i], p.x + 0, p.y + 1, p.z + 0, 1, 1});
            vertices.push_back({m_voxelData[i], p.x + 0, p.y + 1, p.z + 0, 1, 1});
            vertices.push_back({m_voxelData[i], p.x + 1, p.y + 1, p.z + 0, 0, 1});
            vertices.push_back({m_voxelData[i], p.x + 1, p.y + 0, p.z + 0, 0, 0});

            // Left (X-)
            vertices.push_back({m_voxelData[i], p.x + 0, p.y + 0, p.z + 0, 0, 0});
            vertices.push_back({m_voxelData[i], p.x + 0, p.y + 0, p.z + 1, 1, 0});
            vertices.push_back({m_voxelData[i], p.x + 0, p.y + 1, p.z + 1, 1, 1});
            vertices.push_back({m_voxelData[i], p.x + 0, p.y + 1, p.z + 1, 1, 1});
            vertices.push_back({m_voxelData[i], p.x + 0, p.y + 1, p.z + 0, 0, 1});
            vertices.push_back({m_voxelData[i], p.x + 0, p.y + 0, p.z + 0, 0, 0});

            // Right (X+)
            vertices.push_back({m_voxelData[i], p.x + 1, p.y + 0, p.z + 1, 0, 0});
            vertices.push_back({m_voxelData[i], p.x + 1, p.y + 0, p.z + 0, 1, 0});
            vertices.push_back({m_voxelData[i], p.x + 1, p.y + 1, p.z + 0, 1, 1});
            vertices.push_back({m_voxelData[i], p.x + 1, p.y + 1, p.z + 0, 1, 1});
            vertices.push_back({m_voxelData[i], p.x + 1, p.y + 1, p.z + 1, 0, 1});
            vertices.push_back({m_voxelData[i], p.x + 1, p.y + 0, p.z + 1, 0, 0});

            // Top (Y+)
            vertices.push_back({m_voxelData[i], p.x + 0, p.y + 1, p.z + 1, 0, 0});
            vertices.push_back({m_voxelData[i], p.x + 1, p.y + 1, p.z + 1, 1, 0});
            vertices.push_back({m_voxelData[i], p.x + 1, p.y + 1, p.z + 0, 1, 1});
            vertices.push_back({m_voxelData[i], p.x + 1, p.y + 1, p.z + 0, 1, 1});
            vertices.push_back({m_voxelData[i], p.x + 0, p.y + 1, p.z + 0, 0, 1});
            vertices.push_back({m_voxelData[i], p.x + 0, p.y + 1, p.z + 1, 0, 0});

            // Bottom (Y-)
            vertices.push_back({m_voxelData[i], p.x + 0, p.y + 0, p.z + 0, 0, 0});
            vertices.push_back({m_voxelData[i], p.x + 1, p.y + 0, p.z + 0, 1, 0});
            vertices.push_back({m_voxelData[i], p.x + 1, p.y + 0, p.z + 1, 1, 1});
            vertices.push_back({m_voxelData[i], p.x + 1, p.y + 0, p.z + 1, 1, 1});
            vertices.push_back({m_voxelData[i], p.x + 0, p.y + 0, p.z + 1, 0, 1});
            vertices.push_back({m_voxelData[i], p.x + 0, p.y + 0, p.z + 0, 0, 0});
        }

        if (vertices.empty()) return;

        size_t vertexBufferSize = vertices.size() * sizeof(VertexData);

        // Create buffers if they aren't allocated or if they don't have enough space
        // if we already have enough space we can simply reuse the buffer
        if (m_vertexStorageBuffer.Size < vertexBufferSize) {
            FreeBuffersIfAllocated();

            // create buffer for vertices
            m_vertexStorageBuffer = m_renderingManager.GetBufferManager().CreateStorageBuffer(
                vertices.data(), vertexBufferSize, sizeof(VertexData));
        }
    }

    void Chunk::FreeBuffersIfAllocated() {
        if (m_vertexStorageBuffer.Buffer != VK_NULL_HANDLE) m_renderingManager.GetBufferManager().DestroyBuffer(m_vertexStorageBuffer);
        m_vertexStorageBuffer = {};
    }
} // SpireVoxel
