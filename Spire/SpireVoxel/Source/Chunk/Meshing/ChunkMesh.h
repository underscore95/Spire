#pragma once

#include "EngineIncludes.h"
#include "Chunk/VoxelType.h"

namespace SpireVoxel {
    struct VertexData;

    // Represents a generated chunk mesh while it is still CPU side awaiting upload to GPU
    struct ChunkMesh {
        DISABLE_COPY(ChunkMesh)
        DEFAULT_MOVE(ChunkMesh)
        ChunkMesh() = default;

        std::vector<VertexData> Vertices;
        std::vector<VoxelType> VoxelTypes;
    };
} // SpireVoxel
