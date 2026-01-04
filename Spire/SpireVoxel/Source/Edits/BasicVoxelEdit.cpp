#include "BasicVoxelEdit.h"

namespace SpireVoxel {
    BasicVoxelEdit::BasicVoxelEdit(const std::vector<Edit> &edits)
        : m_edits(edits) {
    }

    BasicVoxelEdit::BasicVoxelEdit(const Edit &edit)
        : m_edits({edit}) {
    }

    void BasicVoxelEdit::Apply(VoxelWorld &world) {
        for (const auto &edit : m_edits) {
            Chunk *chunk = world.GetLoadedChunk(VoxelWorld::GetChunkPositionOfVoxel(edit.Position)).get();
            if (!chunk) {
                Spire::warn("Failed to set voxel {} {} {} to {} because chunk {} {} {} wasn't loaded", edit.Position.x, edit.Position.y, edit.Position.z, edit.Type,
                            VoxelWorld::GetChunkPositionOfVoxel(edit.Position).x, VoxelWorld::GetChunkPositionOfVoxel(edit.Position).y,
                            VoxelWorld::GetChunkPositionOfVoxel(edit.Position).z);
                continue;
            }

            std::optional<std::size_t> index = Chunk::GetIndexOfVoxel(chunk->ChunkPosition, edit.Position);
            assert(index);
            chunk->SetVoxel(index.value(), edit.Type);
            NotifyChunkEdit(world, *chunk);
        }
    }
} // SpireVoxel
