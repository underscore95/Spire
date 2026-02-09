#pragma once
#include "IProceduralGenerationProvider.h"

namespace SpireVoxel {
    class EmptyProceduralGenerationProvider : public IProceduralGenerationProvider {
    public:
        void GenerateChunk(VoxelWorld &world, Chunk &chunk) override {
        }
    };
} // SpireVoxel
