#pragma once
#include "Chunk/VoxelWorld.h"
#include "Rendering/VoxelWorldRenderer.h"

namespace SpireVoxel {
    struct Chunk;
    class VoxelWorld;

    // Represents a bulk edit operation which can be applied
    // This API avoids regenerating meshes every single time a voxel is changed if you need to change a voxel many times
    class IVoxelEdit {
    public:
        virtual ~IVoxelEdit() = default;

    public:
        virtual void Apply(VoxelWorld &world) = 0;

    protected:
        static void NotifyChunkEdit(const VoxelWorld& world, Chunk& chunk) {
            world.GetRenderer().NotifyChunkEdited(chunk);
        }
    };
} // SpireVoxel
