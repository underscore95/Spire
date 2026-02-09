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
            // If true, the ray terminated in a chunk with lod scale > 1
            // Raycasting only works in full detail chunks
            bool TerminatedInLODChunk;

            // ReSharper disable once CppNonExplicitConversionOperator
            operator bool() const { return HitAnything; } // NOLINT(*-explicit-constructor)
        };

        [[nodiscard]] static Hit Raycast(VoxelWorld &world, glm::vec3 position, glm::vec3 normalizedForward, float maxRange);

    private:
        // Return true if the chunk at this LOD is
        [[nodiscard]] static bool IsLODChunk(const VoxelWorld &world, glm::vec3 worldPosition);
    };
} // SpireVoxel
