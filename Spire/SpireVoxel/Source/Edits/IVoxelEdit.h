#pragma once
#include "Chunk/VoxelWorld.h"
#include "Rendering/VoxelWorldRenderer.h"

namespace SpireVoxel {
    struct Chunk;
    class VoxelWorld;

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
