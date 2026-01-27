#pragma once
#include "IProceduralGenerationProvider.h"

namespace SpireVoxel {
    // Sets all voxels <= local y 32 to 1 (grass)
    class SimpleProceduralGenerationProvider : public IProceduralGenerationProvider {
    public:
        void GenerateChunk(VoxelWorld &world, Chunk &chunk) override;
    };
} // SpireVoxel
