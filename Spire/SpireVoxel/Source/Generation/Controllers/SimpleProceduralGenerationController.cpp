#include "SimpleProceduralGenerationController.h"

#include "Chunk/VoxelWorld.h"
#include "Utils/IVoxelCamera.h"

namespace SpireVoxel {
    glm::ivec3 GetNextUnloadedChunk(glm::u32 &spiralIndex, VoxelWorld &world, glm::ivec3 cameraChunkPos) {
        cameraChunkPos.y = 0; // Only load chunks at Y=0

        glm::ivec3 spiralCoords = cameraChunkPos + SimpleProceduralGenerationController::GetSpiral(spiralIndex);
        while (world.GetLoadedChunk(spiralCoords)) {
            spiralIndex++;
            spiralCoords = cameraChunkPos + SimpleProceduralGenerationController::GetSpiral(spiralIndex);
        }

        return spiralCoords;
    }

    std::vector<glm::ivec3> SimpleProceduralGenerationController::GetChunkCoordsToLoad(VoxelWorld &world, const IVoxelCamera &camera, glm::u32 maxToLoad) {
        std::vector<glm::ivec3> toLoad;
        toLoad.reserve(maxToLoad);

        glm::ivec3 cameraChunkPos = static_cast<glm::ivec3>(camera.GetPosition()) / 64;
        glm::u32 spiralIndex = 0;

        for (int i = 0; i < maxToLoad; i++) {
            toLoad.push_back(GetNextUnloadedChunk(spiralIndex, world, cameraChunkPos));
        }

        return toLoad;
    }
} // SpireVoxel
