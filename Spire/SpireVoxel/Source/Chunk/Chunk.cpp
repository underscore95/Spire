#include "Chunk.h"

#include "VoxelWorld.h"

namespace SpireVoxel {
    std::vector<VertexData> Chunk::GenerateMesh() {
        std::vector<VertexData> vertices;

        for (size_t i = 0; i < VoxelData.size(); i++) {
            if (VoxelData[i] == 0) continue;

            glm::uvec3 p = SPIRE_VOXEL_INDEX_TO_POSITION(glm::vec3, i);

            // Front (Z+)
            vertices.push_back(PackVertexData(VoxelData[i], p.x + 0, p.y + 0, p.z + 1, 0, 0, SPIRE_VOXEL_FACE_POS_Z));
            vertices.push_back(PackVertexData(VoxelData[i], p.x + 1, p.y + 0, p.z + 1, 1, 0, SPIRE_VOXEL_FACE_POS_Z));
            vertices.push_back(PackVertexData(VoxelData[i], p.x + 1, p.y + 1, p.z + 1, 1, 1, SPIRE_VOXEL_FACE_POS_Z));
            vertices.push_back(PackVertexData(VoxelData[i], p.x + 1, p.y + 1, p.z + 1, 1, 1, SPIRE_VOXEL_FACE_POS_Z));
            vertices.push_back(PackVertexData(VoxelData[i], p.x + 0, p.y + 1, p.z + 1, 0, 1, SPIRE_VOXEL_FACE_POS_Z));
            vertices.push_back(PackVertexData(VoxelData[i], p.x + 0, p.y + 0, p.z + 1, 0, 0, SPIRE_VOXEL_FACE_POS_Z));

            // Back (Z-)
            vertices.push_back(PackVertexData(VoxelData[i], p.x + 1, p.y + 0, p.z + 0, 0, 0, SPIRE_VOXEL_FACE_NEG_Z));
            vertices.push_back(PackVertexData(VoxelData[i], p.x + 0, p.y + 0, p.z + 0, 1, 0, SPIRE_VOXEL_FACE_NEG_Z));
            vertices.push_back(PackVertexData(VoxelData[i], p.x + 0, p.y + 1, p.z + 0, 1, 1, SPIRE_VOXEL_FACE_NEG_Z));
            vertices.push_back(PackVertexData(VoxelData[i], p.x + 0, p.y + 1, p.z + 0, 1, 1, SPIRE_VOXEL_FACE_NEG_Z));
            vertices.push_back(PackVertexData(VoxelData[i], p.x + 1, p.y + 1, p.z + 0, 0, 1, SPIRE_VOXEL_FACE_NEG_Z));
            vertices.push_back(PackVertexData(VoxelData[i], p.x + 1, p.y + 0, p.z + 0, 0, 0, SPIRE_VOXEL_FACE_NEG_Z));

            // Left (X-)
            vertices.push_back(PackVertexData(VoxelData[i], p.x + 0, p.y + 0, p.z + 0, 0, 0, SPIRE_VOXEL_FACE_NEG_X));
            vertices.push_back(PackVertexData(VoxelData[i], p.x + 0, p.y + 0, p.z + 1, 1, 0, SPIRE_VOXEL_FACE_NEG_X));
            vertices.push_back(PackVertexData(VoxelData[i], p.x + 0, p.y + 1, p.z + 1, 1, 1, SPIRE_VOXEL_FACE_NEG_X));
            vertices.push_back(PackVertexData(VoxelData[i], p.x + 0, p.y + 1, p.z + 1, 1, 1, SPIRE_VOXEL_FACE_NEG_X));
            vertices.push_back(PackVertexData(VoxelData[i], p.x + 0, p.y + 1, p.z + 0, 0, 1, SPIRE_VOXEL_FACE_NEG_X));
            vertices.push_back(PackVertexData(VoxelData[i], p.x + 0, p.y + 0, p.z + 0, 0, 0, SPIRE_VOXEL_FACE_NEG_X));

            // Right (X+)
            vertices.push_back(PackVertexData(VoxelData[i], p.x + 1, p.y + 0, p.z + 1, 0, 0, SPIRE_VOXEL_FACE_POS_X));
            vertices.push_back(PackVertexData(VoxelData[i], p.x + 1, p.y + 0, p.z + 0, 1, 0, SPIRE_VOXEL_FACE_POS_X));
            vertices.push_back(PackVertexData(VoxelData[i], p.x + 1, p.y + 1, p.z + 0, 1, 1, SPIRE_VOXEL_FACE_POS_X));
            vertices.push_back(PackVertexData(VoxelData[i], p.x + 1, p.y + 1, p.z + 0, 1, 1, SPIRE_VOXEL_FACE_POS_X));
            vertices.push_back(PackVertexData(VoxelData[i], p.x + 1, p.y + 1, p.z + 1, 0, 1, SPIRE_VOXEL_FACE_POS_X));
            vertices.push_back(PackVertexData(VoxelData[i], p.x + 1, p.y + 0, p.z + 1, 0, 0, SPIRE_VOXEL_FACE_POS_X));

            // Top (Y+)
            vertices.push_back(PackVertexData(VoxelData[i], p.x + 0, p.y + 1, p.z + 1, 0, 0, SPIRE_VOXEL_FACE_POS_Y));
            vertices.push_back(PackVertexData(VoxelData[i], p.x + 1, p.y + 1, p.z + 1, 1, 0, SPIRE_VOXEL_FACE_POS_Y));
            vertices.push_back(PackVertexData(VoxelData[i], p.x + 1, p.y + 1, p.z + 0, 1, 1, SPIRE_VOXEL_FACE_POS_Y));
            vertices.push_back(PackVertexData(VoxelData[i], p.x + 1, p.y + 1, p.z + 0, 1, 1, SPIRE_VOXEL_FACE_POS_Y));
            vertices.push_back(PackVertexData(VoxelData[i], p.x + 0, p.y + 1, p.z + 0, 0, 1, SPIRE_VOXEL_FACE_POS_Y));
            vertices.push_back(PackVertexData(VoxelData[i], p.x + 0, p.y + 1, p.z + 1, 0, 0, SPIRE_VOXEL_FACE_POS_Y));

            // Bottom (Y-)
            vertices.push_back(PackVertexData(VoxelData[i], p.x + 0, p.y + 0, p.z + 0, 0, 0, SPIRE_VOXEL_FACE_NEG_Y));
            vertices.push_back(PackVertexData(VoxelData[i], p.x + 1, p.y + 0, p.z + 0, 1, 0, SPIRE_VOXEL_FACE_NEG_Y));
            vertices.push_back(PackVertexData(VoxelData[i], p.x + 1, p.y + 0, p.z + 1, 1, 1, SPIRE_VOXEL_FACE_NEG_Y));
            vertices.push_back(PackVertexData(VoxelData[i], p.x + 1, p.y + 0, p.z + 1, 1, 1, SPIRE_VOXEL_FACE_NEG_Y));
            vertices.push_back(PackVertexData(VoxelData[i], p.x + 0, p.y + 0, p.z + 1, 0, 1, SPIRE_VOXEL_FACE_NEG_Y));
            vertices.push_back(PackVertexData(VoxelData[i], p.x + 0, p.y + 0, p.z + 0, 0, 0, SPIRE_VOXEL_FACE_NEG_Y));
        }
        return vertices;
    }

    ChunkData Chunk::GenerateChunkData(glm::u32 chunkIndex) const {
        assert(NumVertices > 0);
        return {
            .CPU_DrawCommandParams = {
                .vertexCount = (NumVertices),
                .instanceCount = 1,
                .firstVertex = Allocation.Start / static_cast<glm::u32>(sizeof(VertexData)),
                .firstInstance = chunkIndex
            },
            .ChunkX = ChunkPosition.x,
            .ChunkY = ChunkPosition.y,
            .ChunkZ = ChunkPosition.z
        };
    }
} // SpireVoxel
