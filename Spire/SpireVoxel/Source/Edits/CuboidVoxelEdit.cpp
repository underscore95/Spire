#include "CuboidVoxelEdit.h"

namespace SpireVoxel {
    CuboidVoxelEdit::CuboidVoxelEdit(glm::ivec3 origin, glm::uvec3 size, glm::u32 voxelType)
        : m_voxelType(voxelType),
          m_edits(GenerateEdits(origin, size)),
          m_affectedChunkMeshes(CalculateAffectedChunkMeshes(m_edits)) {
    }

    void CuboidVoxelEdit::Apply(VoxelWorld &world) {
        for (const auto &edit : m_edits) {
            Chunk *chunk = world.GetLoadedChunk(edit.ChunkPosition).get();
            if (!chunk) {
                Spire::warn("Failed to set part of a cuboid in chunk {} {} {} because it wasn't loaded", edit.ChunkPosition.x, edit.ChunkPosition.y, edit.ChunkPosition.z);
                continue;
            }

            static_assert(SPIRE_VOXEL_POSITION_XYZ_TO_INDEX(0, 0, 0) + 1 == SPIRE_VOXEL_POSITION_XYZ_TO_INDEX(0, 0, 1)); // continuous along the z axis
            for (glm::u32 x = edit.RectOrigin.x; x < edit.RectOrigin.x + edit.RectSize.x; x++) {
                for (glm::u32 y = edit.RectOrigin.y; y < edit.RectOrigin.y + edit.RectSize.y; y++) {
                    std::size_t startIndex = SPIRE_VOXEL_POSITION_XYZ_TO_INDEX(x, y, edit.RectOrigin.z);
                    std::size_t endIndex = SPIRE_VOXEL_POSITION_XYZ_TO_INDEX(x, y, edit.RectOrigin.z + edit.RectSize.z);
                    assert(edit.RectOrigin.z + edit.RectSize.z - 1 < SPIRE_VOXEL_CHUNK_SIZE);
                    assert(startIndex < endIndex);
                    assert(endIndex < chunk->VoxelData.size());
                    assert(!chunk->IsCorrupted());
                    chunk->SetVoxels(startIndex, endIndex, m_voxelType);
                    assert(!chunk->IsCorrupted());
                }
            }
        }

        for (const auto &chunkPos : m_affectedChunkMeshes) {
            Chunk *chunk = world.GetLoadedChunk(chunkPos).get();
            if (chunk) {
                NotifyChunkEdit(world, *chunk);
            }
        }
    }

    std::unordered_set<glm::ivec3> CuboidVoxelEdit::CalculateAffectedChunkMeshes(const std::vector<Edit> &edits) {
        std::unordered_set<glm::ivec3> affected;
        for (auto &edit : edits) {
            affected.insert(edit.ChunkPosition);
            if (edit.RectOrigin.x == 0) affected.insert(edit.ChunkPosition + glm::ivec3{-1, 0, 0});
            if (edit.RectOrigin.y == 0) affected.insert(edit.ChunkPosition + glm::ivec3{0, -1, 0});
            if (edit.RectOrigin.z == 0) affected.insert(edit.ChunkPosition + glm::ivec3{0, 0, -1});
            if (edit.RectSize.x + edit.RectOrigin.x >= SPIRE_VOXEL_CHUNK_SIZE - 1) affected.insert(edit.ChunkPosition + glm::ivec3{1, 0, 0});
            if (edit.RectSize.y + edit.RectOrigin.y >= SPIRE_VOXEL_CHUNK_SIZE - 1) affected.insert(edit.ChunkPosition + glm::ivec3{0, 1, 0});
            if (edit.RectSize.z + edit.RectOrigin.z >= SPIRE_VOXEL_CHUNK_SIZE - 1) affected.insert(edit.ChunkPosition + glm::ivec3{0, 0, 1});
        }

        return affected;
    }

    std::vector<CuboidVoxelEdit::Edit> CuboidVoxelEdit::GenerateEdits(glm::ivec3 origin, glm::uvec3 size) {
        glm::ivec3 originChunkPos = VoxelWorld::GetChunkPositionOfVoxel(origin);
        glm::ivec3 endWorldVoxel = origin + glm::ivec3(size);
        glm::ivec3 endChunkPos = VoxelWorld::GetChunkPositionOfVoxel(origin + glm::ivec3(size));

        std::vector<Edit> edits;

        // for each chunk in the rect
        for (glm::i32 chunkX = originChunkPos.x; chunkX <= endChunkPos.x; chunkX++) {
            for (glm::i32 chunkY = originChunkPos.y; chunkY <= endChunkPos.y; chunkY++) {
                for (glm::i32 chunkZ = originChunkPos.z; chunkZ <= endChunkPos.z; chunkZ++) {
                    Edit edit = {};
                    edit.ChunkPosition = {chunkX, chunkY, chunkZ};

                    // if the origin is contained in this chunk, set the rect origin to the origin in chunk-coords
                    // otherwise set the origin to 0 since the origin is before this chunk
                    // this check is done for each axis
                    glm::ivec3 originVoxelInThisChunk = origin - edit.ChunkPosition * SPIRE_VOXEL_CHUNK_SIZE;
                    glm::ivec3 originVoxelOfThisChunk = VoxelWorld::GetWorldVoxelPositionInChunk(edit.ChunkPosition, {0, 0, 0});

                    edit.RectOrigin.x = originVoxelOfThisChunk.x >= origin.x ? 0 : originVoxelInThisChunk.x;
                    edit.RectOrigin.y = originVoxelOfThisChunk.y >= origin.y ? 0 : originVoxelInThisChunk.y;
                    edit.RectOrigin.z = originVoxelOfThisChunk.z >= origin.z ? 0 : originVoxelInThisChunk.z;

                    // same as above but for end voxel
                    glm::ivec3 endVoxelInThisChunk = endWorldVoxel - edit.ChunkPosition * SPIRE_VOXEL_CHUNK_SIZE;
                    edit.RectSize = {SPIRE_VOXEL_CHUNK_SIZE - 1,SPIRE_VOXEL_CHUNK_SIZE - 1, SPIRE_VOXEL_CHUNK_SIZE - 1};
                    glm::ivec3 endVoxelOfThisChunk = VoxelWorld::GetWorldVoxelPositionInChunk(edit.ChunkPosition,
                                                                                              {SPIRE_VOXEL_CHUNK_SIZE - 1,SPIRE_VOXEL_CHUNK_SIZE - 1, SPIRE_VOXEL_CHUNK_SIZE - 1});
                    edit.RectSize.x = endVoxelOfThisChunk.x <= endWorldVoxel.x ? SPIRE_VOXEL_CHUNK_SIZE - edit.RectOrigin.x : endVoxelInThisChunk.x - edit.RectOrigin.x;
                    edit.RectSize.y = endVoxelOfThisChunk.y <= endWorldVoxel.y ? SPIRE_VOXEL_CHUNK_SIZE - edit.RectOrigin.y : endVoxelInThisChunk.y - edit.RectOrigin.y;
                    edit.RectSize.z = endVoxelOfThisChunk.z <= endWorldVoxel.z ? SPIRE_VOXEL_CHUNK_SIZE - edit.RectOrigin.z : endVoxelInThisChunk.z - edit.RectOrigin.z;
                    assert(edit.RectOrigin.x + edit.RectSize.x <= SPIRE_VOXEL_CHUNK_SIZE);
                    assert(edit.RectOrigin.y + edit.RectSize.y <= SPIRE_VOXEL_CHUNK_SIZE);
                    assert(edit.RectOrigin.z + edit.RectSize.z <= SPIRE_VOXEL_CHUNK_SIZE);

                    edits.push_back(edit);
                }
            }
        }

        edits.shrink_to_fit();
        return edits;
    }
} // SpireVoxel
