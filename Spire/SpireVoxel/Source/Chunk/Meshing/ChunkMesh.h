#pragma once

#include "EngineIncludes.h"
#include "Chunk/VoxelType.h"
#include "../Assets/Shaders/ShaderInfo.h"

namespace SpireVoxel {
    struct VertexData;

    // Represents a generated chunk mesh while it is still CPU side awaiting upload to GPU
    struct ChunkMesh {
        DISABLE_COPY(ChunkMesh)

        DEFAULT_MOVE(ChunkMesh)

        ChunkMesh() = default;

        // Each chunk is rendered as 6 separate meshes, one per face
        std::array<std::vector<VertexData>, SPIRE_VOXEL_NUM_FACES> Vertices;
        std::vector<VoxelType> VoxelTypes;
        std::vector<glm::u32> AOData;
        glm::u32 AODataValueCount = 0; // since each AO value is smaller than a u32

        [[nodiscard]] int CountVertices() const {
            int count = 0;
            for (const auto &vec : Vertices) {
                count += vec.size();
            }
            return count;
        }

        [[nodiscard]] std::array<glm::u32, SPIRE_VOXEL_NUM_FACES> GetVertexCounts() const {
            return {
                static_cast<glm::u32>(Vertices[0].size()),
                static_cast<glm::u32>(Vertices[1].size()),
                static_cast<glm::u32>(Vertices[2].size()),
                static_cast<glm::u32>(Vertices[3].size()),
                static_cast<glm::u32>(Vertices[4].size()),
                static_cast<glm::u32>(Vertices[5].size()),
            };
        }
    };
} // SpireVoxel
