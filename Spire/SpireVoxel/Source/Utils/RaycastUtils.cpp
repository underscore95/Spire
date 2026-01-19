#include "RaycastUtils.h"
#include <glm/gtc/epsilon.hpp>
#include "../../Assets/Shaders/ShaderInfo.h"
#include "Chunk/VoxelWorld.h"

namespace SpireVoxel {
    RaycastUtils::Hit RaycastUtils::Raycast(VoxelWorld &world,
                                            glm::vec3 position,
                                            glm::vec3 normalizedForward,
                                            float maxRange) {
        // voxel traversal algorithm by https://doi.org/10.2312/egtp.19871000
        glm::vec3 origin = position;

        // assert forward is normalized
        assert(
            !glm::epsilonEqual(normalizedForward.x, 0.0f, glm::epsilon<float>())
            || !glm::epsilonEqual(normalizedForward.y, 0.0f, glm::epsilon<float>())
            || !glm::epsilonEqual(normalizedForward.z, 0.0f, glm::epsilon<float>())
        );
        glm::vec3 dir = normalizedForward;
        glm::ivec3 voxel = glm::floor(origin);
        glm::ivec3 step = {
            dir.x > 0 ? 1 : -1,
            dir.y > 0 ? 1 : -1,
            dir.z > 0 ? 1 : -1
        };

        glm::vec3 tMax;
        glm::vec3 tDelta;

        for (int i = 0; i < 3; i++) {
            float o = origin[i];
            float d = dir[i];
            int s = step[i];
            float vb = static_cast<float>(voxel[i]) + (s > 0 ? 1.0f : 0.0f);
            tMax[i] = (vb - o) / d;
            tDelta[i] = 1.0f / std::abs(d);
        }

        auto maxDist = maxRange;
        Hit hit = {.HitAnything = false};

        int lastAxis = -1;

        while (true) {
            glm::vec3 voxelCenter = glm::vec3(voxel) + 0.5f;
            if (glm::distance(origin, voxelCenter) > maxDist) return hit;

            if (world.IsVoxelAt(voxel)) {
                hit.HitAnything = true;
                hit.VoxelPosition = voxel;
                if (lastAxis == 0) hit.Face = step.x > 0 ? SPIRE_VOXEL_FACE_NEG_X : SPIRE_VOXEL_FACE_POS_X;
                if (lastAxis == 1) hit.Face = step.y > 0 ? SPIRE_VOXEL_FACE_NEG_Y : SPIRE_VOXEL_FACE_POS_Y;
                if (lastAxis == 2) hit.Face = step.z > 0 ? SPIRE_VOXEL_FACE_NEG_Z : SPIRE_VOXEL_FACE_POS_Z;
                return hit;
            }

            if (tMax.x < tMax.y) {
                if (tMax.x < tMax.z) {
                    voxel.x += step.x;
                    tMax.x += tDelta.x;
                    lastAxis = 0;
                } else {
                    voxel.z += step.z;
                    tMax.z += tDelta.z;
                    lastAxis = 2;
                }
            } else {
                if (tMax.y < tMax.z) {
                    voxel.y += step.y;
                    tMax.y += tDelta.y;
                    lastAxis = 1;
                } else {
                    voxel.z += step.z;
                    tMax.z += tDelta.z;
                    lastAxis = 2;
                }
            }
        }
    }
} // SpireVoxel
