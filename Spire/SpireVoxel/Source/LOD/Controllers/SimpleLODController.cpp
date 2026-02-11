#include "SimpleLODController.h"

#include "Chunk/Chunk.h"
#include "Chunk/VoxelWorld.h"

namespace SpireVoxel {
    SimpleLODController::SimpleLODController(VoxelWorld &world)
        : m_world(world) {
    }

    glm::u32 SimpleLODController::GetLODLevel(float chunkDistance) {
        if (chunkDistance > 8) return 2;
        return 1;
    }

    std::vector<glm::ivec3> SimpleLODController::GetChunkCoordsBatch(VoxelWorld &world, const IVoxelCamera &camera, glm::u32 maxBatchSize) {
        std::vector<glm::ivec3> chunks;
    }

    SimpleLODController::LODChunk SimpleLODController::GetBestLOD(VoxelWorld &world, glm::ivec3 coords, glm::u32 desiredLODLevel) {
        if (desiredLODLevel <= 1) return LODChunk{1, coords};

        glm::ivec3 actualChunk = coords;
        for (int i = 0; i < glm::ivec3::length(); i++) {
            if (actualChunk[i] < 0) actualChunk -= (desiredLODLevel - 1);
        }

        // check if there's no LOD 2 or higher chunks in the way
        for (int x = coords.x; x < coords.x + desiredLODLevel; x++) {
            for (int y = coords.y; y < coords.y + desiredLODLevel; y++) {
                for (int z = coords.z; z < coords.z + desiredLODLevel; z++) {
                    Chunk *chunk = world.GetLODManager().TryGetLODChunk({x, y, z});
                    if (chunk && chunk->LOD.Scale > 1) {
                        return GetBestLOD(world, coords, desiredLODLevel - 1); // something in the way, lets try a different lod
                    }
                }
            }
        }

        return {desiredLODLevel, actualChunk};
    }

    Chunk *SimpleLODController::FindFurthestLOD1Chunk(glm::ivec3 cameraChunk, std::vector<glm::ivec3> &ignore) const {
        Chunk *furthest = nullptr;
        glm::u32 furthestDistSqr = 0;

        for (auto &[coords, chunk] : m_world) {
            if (chunk->LOD.Scale != 1 || std::ranges::contains(ignore, chunk->ChunkPosition)) continue;
            glm::u32 distSqr(glm::dot(cameraChunk - coords, cameraChunk - coords));
            if (!furthest || furthestDistSqr < distSqr) {
                furthest = chunk.get();
                furthestDistSqr = distSqr;
            }
        }

        return furthest;
    }
} // SpireVoxel
