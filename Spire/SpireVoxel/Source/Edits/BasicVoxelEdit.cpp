#include "BasicVoxelEdit.h"

namespace SpireVoxel {
    BasicVoxelEdit::BasicVoxelEdit(const std::vector<Edit> &edits)
        : m_edits(edits) {
    }

    BasicVoxelEdit::BasicVoxelEdit(const Edit &edit)
        : m_edits({edit}) {
    }

    BasicVoxelEdit::BasicVoxelEdit(glm::ivec3 position, glm::u32 type) : m_edits({{position, type}}) {
    }

    void BasicVoxelEdit::Apply(VoxelWorld &world) {
        for (const auto &edit : m_edits) {
            Chunk *chunk = world.GetLoadedChunk(world.GetChunkPositionOfVoxel(edit.Position));
            if (!chunk) {
                Spire::warn("Failed to set voxel {} {} {} to {} because chunk {} {} {} wasn't loaded", edit.Position.x, edit.Position.y, edit.Position.z, edit.Type,
                            world.GetChunkPositionOfVoxel(edit.Position).x, world.GetChunkPositionOfVoxel(edit.Position).y, world.GetChunkPositionOfVoxel(edit.Position).z);
                continue;
            }

            std::optional<std::size_t> index = Chunk::GetIndexOfVoxel(chunk->ChunkPosition, edit.Position);
            assert(index);
            chunk->SetVoxel(index.value(), edit.Type);
            NotifyChunkEdit(world, *chunk);
        }
    }
} // SpireVoxel
