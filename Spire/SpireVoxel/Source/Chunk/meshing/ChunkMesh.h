#pragma once

#include "EngineIncludes.h"

namespace SpireVoxel {
    struct VertexData;

    // Represents a generated chunk mesh while it is still CPU side awaiting upload to GPU
    struct ChunkMesh {
        std::vector<VertexData> Vertices;
    };
} // SpireVoxel
