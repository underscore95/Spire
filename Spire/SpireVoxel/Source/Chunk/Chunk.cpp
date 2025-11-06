#include "Chunk.h"

namespace SpireVoxel {
    Chunk::Chunk(Spire::RenderingManager &renderingManager,
                 glm::ivec3 chunkPosition)
        : m_renderingManager(renderingManager),
          m_chunkPosition(chunkPosition) {
        RegenerateMesh();
    }

    Chunk::~Chunk() {
        FreeBuffersIfAllocated();
    }

    void Chunk::SetVoxel(glm::vec3 pos, std::int32_t type) {
        m_voxelData[SPIRE_VOXEL_POSITION_TO_INDEX(pos)] = type;
        RegenerateMesh();
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
        RegenerateMesh();
    }

    void Chunk::CmdBindIndexBuffer(VkCommandBuffer commandBuffer) const {
        if (m_numIndices == VOXEL_TYPE_AIR) return;

        static_assert(sizeof(MESH_INDEX_TYPE) == sizeof(glm::u32)); // update index type below if changed!
        vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer.Buffer, 0, VK_INDEX_TYPE_UINT32);
    }

    void Chunk::CmdRender(VkCommandBuffer commandBuffer) const {
        if (m_numIndices == 0) return;

        vkCmdDrawIndexed(commandBuffer, m_numIndices, 1, 0, 0, 0);
    }

    Spire::Descriptor Chunk::GetDescriptor(glm::u32 binding) const {
        return {
            .ResourceType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .Binding = binding,
            .Stages = VK_SHADER_STAGE_VERTEX_BIT,
            .NumResources = 1,
            .ResourcePtrs = &m_vertexStorageBuffer
        };
    }

    void Chunk::RegenerateMesh() {
        FreeBuffersIfAllocated();

        // generate vertices & indices
        std::vector<VertexData> vertices;
        std::vector<MESH_INDEX_TYPE> indices;

        for (size_t i = 0; i < m_voxelData.size(); i++) {
            if (m_voxelData[i] == 0) continue;

            glm::vec3 p = SPIRE_VOXEL_INDEX_TO_POSITION(glm::vec3, i) + glm::vec3(m_chunkPosition * SPIRE_VOXEL_CHUNK_SIZE);
            uint32_t start = static_cast<uint32_t>(vertices.size());

            // Front (Z+)
            vertices.push_back({m_voxelData[i], p.x + 0, p.y + 0, p.z + 1, 0, 0});
            vertices.push_back({m_voxelData[i], p.x + 1, p.y + 0, p.z + 1, 1, 0});
            vertices.push_back({m_voxelData[i], p.x + 1, p.y + 1, p.z + 1, 1, 1});
            vertices.push_back({m_voxelData[i], p.x + 0, p.y + 1, p.z + 1, 0, 1});

            // Back (Z-)
            vertices.push_back({m_voxelData[i], p.x + 1, p.y + 0, p.z + 0, 0, 0});
            vertices.push_back({m_voxelData[i], p.x + 0, p.y + 0, p.z + 0, 1, 0});
            vertices.push_back({m_voxelData[i], p.x + 0, p.y + 1, p.z + 0, 1, 1});
            vertices.push_back({m_voxelData[i], p.x + 1, p.y + 1, p.z + 0, 0, 1});

            // Left (X-)
            vertices.push_back({m_voxelData[i], p.x + 0, p.y + 0, p.z + 0, 0, 0});
            vertices.push_back({m_voxelData[i], p.x + 0, p.y + 0, p.z + 1, 1, 0});
            vertices.push_back({m_voxelData[i], p.x + 0, p.y + 1, p.z + 1, 1, 1});
            vertices.push_back({m_voxelData[i], p.x + 0, p.y + 1, p.z + 0, 0, 1});

            // Right (X+)
            vertices.push_back({m_voxelData[i], p.x + 1, p.y + 0, p.z + 1, 0, 0});
            vertices.push_back({m_voxelData[i], p.x + 1, p.y + 0, p.z + 0, 1, 0});
            vertices.push_back({m_voxelData[i], p.x + 1, p.y + 1, p.z + 0, 1, 1});
            vertices.push_back({m_voxelData[i], p.x + 1, p.y + 1, p.z + 1, 0, 1});

            // Top (Y+)
            vertices.push_back({m_voxelData[i], p.x + 0, p.y + 1, p.z + 1, 0, 0});
            vertices.push_back({m_voxelData[i], p.x + 1, p.y + 1, p.z + 1, 1, 0});
            vertices.push_back({m_voxelData[i], p.x + 1, p.y + 1, p.z + 0, 1, 1});
            vertices.push_back({m_voxelData[i], p.x + 0, p.y + 1, p.z + 0, 0, 1});

            // Bottom (Y-)
            vertices.push_back({m_voxelData[i], p.x + 0, p.y + 0, p.z + 0, 0, 0});
            vertices.push_back({m_voxelData[i], p.x + 1, p.y + 0, p.z + 0, 1, 0});
            vertices.push_back({m_voxelData[i], p.x + 1, p.y + 0, p.z + 1, 1, 1});
            vertices.push_back({m_voxelData[i], p.x + 0, p.y + 0, p.z + 1, 0, 1});

            // Indices
            for (int f = 0; f < 6; f++) {
                uint32_t s = start + f * 4;
                indices.push_back(s + 0);
                indices.push_back(s + 1);
                indices.push_back(s + 2);
                indices.push_back(s + 2);
                indices.push_back(s + 3);
                indices.push_back(s + 0);
            }
        }

        m_numIndices = indices.size();
        if (m_numIndices == 0) return;

        // create buffers
        size_t vertexBufferSize = vertices.size() * sizeof(VertexData);

        // create buffer for vertices
        m_vertexStorageBuffer = m_renderingManager.GetBufferManager().CreateStorageBuffer(
            vertices.data(), vertexBufferSize, sizeof(VertexData));

        // create index buffer
        m_indexBuffer = m_renderingManager.GetBufferManager().CreateIndexBuffer(
            sizeof(MESH_INDEX_TYPE), indices.data(), indices.size()
        );
    }

    void Chunk::FreeBuffersIfAllocated() {
        if (m_vertexStorageBuffer.Buffer != VK_NULL_HANDLE) m_renderingManager.GetBufferManager().DestroyBuffer(m_vertexStorageBuffer);
        m_vertexStorageBuffer = {};

        if (m_indexBuffer.Buffer != VK_NULL_HANDLE) m_renderingManager.GetBufferManager().DestroyBuffer(m_indexBuffer);
        m_indexBuffer = {};
    }
} // SpireVoxel
