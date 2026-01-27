#pragma once
#include "Chunk/Chunk.h"

namespace SpireVoxel {
    // Generate the voxel data for a chunk
    // Method will be called on multiple threads at the same time
    class IProceduralGenerationProvider {
    public:
        virtual ~IProceduralGenerationProvider() = default;

    public:
        virtual void GenerateChunk(VoxelWorld &world, Chunk &chunk) = 0;
    };
} // SpireVoxel
