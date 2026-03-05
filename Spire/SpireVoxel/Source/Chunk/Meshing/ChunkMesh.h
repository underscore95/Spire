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

        // There is 6 meshes, one for each face, how many vertices does each face contain?
        std::array<glm::u32, SPIRE_VOXEL_NUM_FACES> VertexCounts;
        std::vector<VertexData> Vertices;
        std::vector<VoxelType> VoxelTypes;
        std::vector<glm::u32> AOData;
        glm::u32 AODataValueCount = 0; // since each AO value is smaller than a u32
    };
} // SpireVoxel
