#include "Chunk.h"

#include "VoxelWorld.h"

namespace SpireVoxel {
    std::vector<VertexData> Chunk::GenerateMesh() {
        std::vector<VertexData> vertices;

        for (size_t i = 0; i < VoxelData.size(); i++) {
            if (VoxelData[i] == 0) continue;

            glm::vec3 p = SPIRE_VOXEL_INDEX_TO_POSITION(glm::vec3, i) + glm::vec3(ChunkPosition * SPIRE_VOXEL_CHUNK_SIZE);

            // Front (Z+)
            vertices.push_back({VoxelData[i], p.x + 0, p.y + 0, p.z + 1, 0, 0});
            vertices.push_back({VoxelData[i], p.x + 1, p.y + 0, p.z + 1, 1, 0});
            vertices.push_back({VoxelData[i], p.x + 1, p.y + 1, p.z + 1, 1, 1});
            vertices.push_back({VoxelData[i], p.x + 1, p.y + 1, p.z + 1, 1, 1});
            vertices.push_back({VoxelData[i], p.x + 0, p.y + 1, p.z + 1, 0, 1});
            vertices.push_back({VoxelData[i], p.x + 0, p.y + 0, p.z + 1, 0, 0});

            // Back (Z-)
            vertices.push_back({VoxelData[i], p.x + 1, p.y + 0, p.z + 0, 0, 0});
            vertices.push_back({VoxelData[i], p.x + 0, p.y + 0, p.z + 0, 1, 0});
            vertices.push_back({VoxelData[i], p.x + 0, p.y + 1, p.z + 0, 1, 1});
            vertices.push_back({VoxelData[i], p.x + 0, p.y + 1, p.z + 0, 1, 1});
            vertices.push_back({VoxelData[i], p.x + 1, p.y + 1, p.z + 0, 0, 1});
            vertices.push_back({VoxelData[i], p.x + 1, p.y + 0, p.z + 0, 0, 0});

            // Left (X-)
            vertices.push_back({VoxelData[i], p.x + 0, p.y + 0, p.z + 0, 0, 0});
            vertices.push_back({VoxelData[i], p.x + 0, p.y + 0, p.z + 1, 1, 0});
            vertices.push_back({VoxelData[i], p.x + 0, p.y + 1, p.z + 1, 1, 1});
            vertices.push_back({VoxelData[i], p.x + 0, p.y + 1, p.z + 1, 1, 1});
            vertices.push_back({VoxelData[i], p.x + 0, p.y + 1, p.z + 0, 0, 1});
            vertices.push_back({VoxelData[i], p.x + 0, p.y + 0, p.z + 0, 0, 0});

            // Right (X+)
            vertices.push_back({VoxelData[i], p.x + 1, p.y + 0, p.z + 1, 0, 0});
            vertices.push_back({VoxelData[i], p.x + 1, p.y + 0, p.z + 0, 1, 0});
            vertices.push_back({VoxelData[i], p.x + 1, p.y + 1, p.z + 0, 1, 1});
            vertices.push_back({VoxelData[i], p.x + 1, p.y + 1, p.z + 0, 1, 1});
            vertices.push_back({VoxelData[i], p.x + 1, p.y + 1, p.z + 1, 0, 1});
            vertices.push_back({VoxelData[i], p.x + 1, p.y + 0, p.z + 1, 0, 0});

            // Top (Y+)
            vertices.push_back({VoxelData[i], p.x + 0, p.y + 1, p.z + 1, 0, 0});
            vertices.push_back({VoxelData[i], p.x + 1, p.y + 1, p.z + 1, 1, 0});
            vertices.push_back({VoxelData[i], p.x + 1, p.y + 1, p.z + 0, 1, 1});
            vertices.push_back({VoxelData[i], p.x + 1, p.y + 1, p.z + 0, 1, 1});
            vertices.push_back({VoxelData[i], p.x + 0, p.y + 1, p.z + 0, 0, 1});
            vertices.push_back({VoxelData[i], p.x + 0, p.y + 1, p.z + 1, 0, 0});

            // Bottom (Y-)
            vertices.push_back({VoxelData[i], p.x + 0, p.y + 0, p.z + 0, 0, 0});
            vertices.push_back({VoxelData[i], p.x + 1, p.y + 0, p.z + 0, 1, 0});
            vertices.push_back({VoxelData[i], p.x + 1, p.y + 0, p.z + 1, 1, 1});
            vertices.push_back({VoxelData[i], p.x + 1, p.y + 0, p.z + 1, 1, 1});
            vertices.push_back({VoxelData[i], p.x + 0, p.y + 0, p.z + 1, 0, 1});
            vertices.push_back({VoxelData[i], p.x + 0, p.y + 0, p.z + 0, 0, 0});
        }
        return vertices;
    }
} // SpireVoxel
