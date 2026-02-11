#include "SimpleProceduralGenerationController.h"

#include <complex.h>
#include <complex.h>

#include "Chunk/VoxelWorld.h"
#include "Edits/BasicVoxelEdit.h"
#include "Edits/BasicVoxelEdit.h"
#include "Edits/BasicVoxelEdit.h"
#include "Edits/BasicVoxelEdit.h"
#include "Edits/BasicVoxelEdit.h"
#include "Edits/BasicVoxelEdit.h"
#include "Utils/IVoxelCamera.h"

namespace SpireVoxel {
    SimpleProceduralGenerationController::SimpleProceduralGenerationController(
        glm::u32 viewDistance,
        glm::ivec2 yRange)
        : m_viewDistance(viewDistance),
          m_yRange(yRange) {
    }

    std::vector<glm::ivec3> SimpleProceduralGenerationController::GetChunkCoordsBatch(VoxelWorld &world, const IVoxelCamera &camera, glm::u32 maxToLoad) {
        std::vector<glm::ivec3> toLoad;
        toLoad.reserve(maxToLoad);

        glm::ivec3 cameraChunkPos = static_cast<glm::ivec3>(camera.GetPosition()) / 64;
        glm::u32 spiralIndex = 0;

        for (int i = 0; i < maxToLoad; i++) {
            glm::ivec3 chunk = GetNextUnloadedChunk(spiralIndex, world, cameraChunkPos, toLoad);
            if (glm::distance(static_cast<glm::vec3>(cameraChunkPos), static_cast<glm::vec3>(chunk)) >= m_viewDistance) continue;
            toLoad.push_back(chunk);
        }

        return toLoad;
    }


    glm::ivec3 SimpleProceduralGenerationController::GetNextUnloadedChunk(glm::u32 &spiralIndex, VoxelWorld &world, glm::ivec3 cameraChunkPos,
                                                                          const std::vector<glm::ivec3> &alreadyFound) const {
        cameraChunkPos.y = 0;

        while (true) {
            glm::ivec3 base = cameraChunkPos + GetSpiral(spiralIndex);

            for (int y = m_yRange.x; y <= m_yRange.y; y++) {
                glm::ivec3 coords = base;
                coords.y = y;

                if (!world.GetLODManager().TryGetLODChunk(coords) && std::ranges::find(alreadyFound, coords) == alreadyFound.end())
                    return coords;
            }

            spiralIndex++;
        }
    }
} // SpireVoxel
