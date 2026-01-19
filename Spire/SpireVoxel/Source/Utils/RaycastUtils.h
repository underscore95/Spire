#pragma once

#include "EngineIncludes.h"

namespace SpireVoxel {
    class VoxelWorld;

    // Voxel raycast utility
    // traverses the voxel world until it hits a voxel
    class RaycastUtils {
    public:
        RaycastUtils() = delete;

    public:
        struct Hit {
            bool HitAnything;
            glm::ivec3 VoxelPosition;
            glm::u32 Face;

            // ReSharper disable once CppNonExplicitConversionOperator
            operator bool() const { return HitAnything; } // NOLINT(*-explicit-constructor)
        };

        [[nodiscard]] static Hit Raycast(VoxelWorld &world, glm::vec3 position, glm::vec3 normalizedForward, float maxRange);
    };
} // SpireVoxel
