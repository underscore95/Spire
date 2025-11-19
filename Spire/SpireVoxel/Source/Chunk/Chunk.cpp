#include "Chunk.h"

#include "VoxelWorld.h"

namespace SpireVoxel {
    std::vector<VertexData> Chunk::GenerateMesh() {
        std::vector<VertexData> vertices;

        for (size_t i = 0; i < VoxelData.size(); i++) {
            if (VoxelData[i] == 0) continue;

            glm::vec3 p = SPIRE_VOXEL_INDEX_TO_POSITION(glm::vec3, i) + glm::vec3(ChunkPosition * SPIRE_VOXEL_CHUNK_SIZE);

            // Front (Z+)
            vertices.push_back({VoxelData[i], p.x + 0, p.y + 0, p.z + 1, 0, 0, SPIRE_VOXEL_FACE_POS_Z});
            vertices.push_back({VoxelData[i], p.x + 1, p.y + 0, p.z + 1, 1, 0, SPIRE_VOXEL_FACE_POS_Z});
            vertices.push_back({VoxelData[i], p.x + 1, p.y + 1, p.z + 1, 1, 1, SPIRE_VOXEL_FACE_POS_Z});
            vertices.push_back({VoxelData[i], p.x + 1, p.y + 1, p.z + 1, 1, 1, SPIRE_VOXEL_FACE_POS_Z});
            vertices.push_back({VoxelData[i], p.x + 0, p.y + 1, p.z + 1, 0, 1, SPIRE_VOXEL_FACE_POS_Z});
            vertices.push_back({VoxelData[i], p.x + 0, p.y + 0, p.z + 1, 0, 0, SPIRE_VOXEL_FACE_POS_Z});

            // Back (Z-)
            vertices.push_back({VoxelData[i], p.x + 1, p.y + 0, p.z + 0, 0, 0, SPIRE_VOXEL_FACE_NEG_Z});
            vertices.push_back({VoxelData[i], p.x + 0, p.y + 0, p.z + 0, 1, 0, SPIRE_VOXEL_FACE_NEG_Z});
            vertices.push_back({VoxelData[i], p.x + 0, p.y + 1, p.z + 0, 1, 1, SPIRE_VOXEL_FACE_NEG_Z});
            vertices.push_back({VoxelData[i], p.x + 0, p.y + 1, p.z + 0, 1, 1, SPIRE_VOXEL_FACE_NEG_Z});
            vertices.push_back({VoxelData[i], p.x + 1, p.y + 1, p.z + 0, 0, 1, SPIRE_VOXEL_FACE_NEG_Z});
            vertices.push_back({VoxelData[i], p.x + 1, p.y + 0, p.z + 0, 0, 0, SPIRE_VOXEL_FACE_NEG_Z});

            // Left (X-)
            vertices.push_back({VoxelData[i], p.x + 0, p.y + 0, p.z + 0, 0, 0, SPIRE_VOXEL_FACE_NEG_X});
            vertices.push_back({VoxelData[i], p.x + 0, p.y + 0, p.z + 1, 1, 0, SPIRE_VOXEL_FACE_NEG_X});
            vertices.push_back({VoxelData[i], p.x + 0, p.y + 1, p.z + 1, 1, 1, SPIRE_VOXEL_FACE_NEG_X});
            vertices.push_back({VoxelData[i], p.x + 0, p.y + 1, p.z + 1, 1, 1, SPIRE_VOXEL_FACE_NEG_X});
            vertices.push_back({VoxelData[i], p.x + 0, p.y + 1, p.z + 0, 0, 1, SPIRE_VOXEL_FACE_NEG_X});
            vertices.push_back({VoxelData[i], p.x + 0, p.y + 0, p.z + 0, 0, 0, SPIRE_VOXEL_FACE_NEG_X});

            // Right (X+)
            vertices.push_back({VoxelData[i], p.x + 1, p.y + 0, p.z + 1, 0, 0, SPIRE_VOXEL_FACE_POS_X});
            vertices.push_back({VoxelData[i], p.x + 1, p.y + 0, p.z + 0, 1, 0, SPIRE_VOXEL_FACE_POS_X});
            vertices.push_back({VoxelData[i], p.x + 1, p.y + 1, p.z + 0, 1, 1, SPIRE_VOXEL_FACE_POS_X});
            vertices.push_back({VoxelData[i], p.x + 1, p.y + 1, p.z + 0, 1, 1, SPIRE_VOXEL_FACE_POS_X});
            vertices.push_back({VoxelData[i], p.x + 1, p.y + 1, p.z + 1, 0, 1, SPIRE_VOXEL_FACE_POS_X});
            vertices.push_back({VoxelData[i], p.x + 1, p.y + 0, p.z + 1, 0, 0, SPIRE_VOXEL_FACE_POS_X});

            // Top (Y+)
            vertices.push_back({VoxelData[i], p.x + 0, p.y + 1, p.z + 1, 0, 0, SPIRE_VOXEL_FACE_POS_Y});
            vertices.push_back({VoxelData[i], p.x + 1, p.y + 1, p.z + 1, 1, 0, SPIRE_VOXEL_FACE_POS_Y});
            vertices.push_back({VoxelData[i], p.x + 1, p.y + 1, p.z + 0, 1, 1, SPIRE_VOXEL_FACE_POS_Y});
            vertices.push_back({VoxelData[i], p.x + 1, p.y + 1, p.z + 0, 1, 1, SPIRE_VOXEL_FACE_POS_Y});
            vertices.push_back({VoxelData[i], p.x + 0, p.y + 1, p.z + 0, 0, 1, SPIRE_VOXEL_FACE_POS_Y});
            vertices.push_back({VoxelData[i], p.x + 0, p.y + 1, p.z + 1, 0, 0, SPIRE_VOXEL_FACE_POS_Y});

            // Bottom (Y-)
            vertices.push_back({VoxelData[i], p.x + 0, p.y + 0, p.z + 0, 0, 0, SPIRE_VOXEL_FACE_NEG_Y});
            vertices.push_back({VoxelData[i], p.x + 1, p.y + 0, p.z + 0, 1, 0, SPIRE_VOXEL_FACE_NEG_Y});
            vertices.push_back({VoxelData[i], p.x + 1, p.y + 0, p.z + 1, 1, 1, SPIRE_VOXEL_FACE_NEG_Y});
            vertices.push_back({VoxelData[i], p.x + 1, p.y + 0, p.z + 1, 1, 1, SPIRE_VOXEL_FACE_NEG_Y});
            vertices.push_back({VoxelData[i], p.x + 0, p.y + 0, p.z + 1, 0, 1, SPIRE_VOXEL_FACE_NEG_Y});
            vertices.push_back({VoxelData[i], p.x + 0, p.y + 0, p.z + 0, 0, 0, SPIRE_VOXEL_FACE_NEG_Y});
        }
        return vertices;
    }
} // SpireVoxel
