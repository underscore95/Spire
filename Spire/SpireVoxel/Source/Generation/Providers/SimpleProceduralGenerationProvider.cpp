#include "SimpleProceduralGenerationProvider.h"

#include "Edits/CuboidVoxelEdit.h"

namespace SpireVoxel {
    void SimpleProceduralGenerationProvider::GenerateChunk(VoxelWorld &world, Chunk &chunk) {
        glm::ivec3 chunkOrigin = VoxelWorld::GetWorldVoxelPositionInChunk(chunk.ChunkPosition, {0,0,0});
        CuboidVoxelEdit(chunkOrigin, {64, 32, 64}, 1).Apply(world);
    }
} // SpireVoxel