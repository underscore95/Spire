#include "Chunk.h"

#include "GreedyMeshingBitmask.h"
#include "VoxelWorld.h"

namespace SpireVoxel {
    glm::u32 GetAdjacentVoxelType(const Chunk &chunk, glm::ivec3 position, glm::u32 face) {
        glm::ivec3 queryChunkPosition = chunk.ChunkPosition;
        glm::ivec3 queryPosition = position + FaceToDirection(face);

        assert(queryPosition.x >= -1 && queryPosition.x <= SPIRE_VOXEL_CHUNK_SIZE);
        assert(queryPosition.y >= -1 && queryPosition.y <= SPIRE_VOXEL_CHUNK_SIZE);
        assert(queryPosition.z >= -1 && queryPosition.z <= SPIRE_VOXEL_CHUNK_SIZE);

        if (queryPosition.x < 0) {
            queryPosition.x += SPIRE_VOXEL_CHUNK_SIZE;
            queryChunkPosition.x--;
        }
        if (queryPosition.x >= SPIRE_VOXEL_CHUNK_SIZE) {
            queryPosition.x -= SPIRE_VOXEL_CHUNK_SIZE;
            queryChunkPosition.x++;
        }

        if (queryPosition.y < 0) {
            queryPosition.y += SPIRE_VOXEL_CHUNK_SIZE;
            queryChunkPosition.y--;
        }
        if (queryPosition.y >= SPIRE_VOXEL_CHUNK_SIZE) {
            queryPosition.y -= SPIRE_VOXEL_CHUNK_SIZE;
            queryChunkPosition.y++;
        }

        if (queryPosition.z < 0) {
            queryPosition.z += SPIRE_VOXEL_CHUNK_SIZE;
            queryChunkPosition.z--;
        }
        if (queryPosition.z >= SPIRE_VOXEL_CHUNK_SIZE) {
            queryPosition.z -= SPIRE_VOXEL_CHUNK_SIZE;
            queryChunkPosition.z++;
        }

        const Chunk *queryChunk = &chunk;
        if (queryChunkPosition != chunk.ChunkPosition) {
            queryChunk = chunk.World.GetLoadedChunk(queryChunkPosition);
            if (!queryChunk) return VOXEL_TYPE_AIR;
        }
        return queryChunk->VoxelData[SPIRE_VOXEL_POSITION_TO_INDEX(queryPosition)];
    }

    void PushFace(std::vector<VertexData> &vertices, glm::u32 type, glm::u32 face, glm::uvec3 p, glm::u32 width, glm::u32 height) {
        assert(width > 0);
        assert(height > 0);
        //  Spire::info("pushing {}x{}",width,height);
        const glm::u32 w = width;
        const glm::u32 h = height;
        switch (face) {
            case SPIRE_VOXEL_FACE_POS_Z:
                vertices.push_back(PackVertexData(type, p.x + 0, p.y + 0, p.z + 1, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_POS_Z));
                vertices.push_back(PackVertexData(type, p.x + 0, p.y + h, p.z + 1, VoxelVertexPosition::THREE, SPIRE_VOXEL_FACE_POS_Z));
                vertices.push_back(PackVertexData(type, p.x + w, p.y + h, p.z + 1, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_POS_Z));
                vertices.push_back(PackVertexData(type, p.x + w, p.y + h, p.z + 1, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_POS_Z));
                vertices.push_back(PackVertexData(type, p.x + w, p.y + 0, p.z + 1, VoxelVertexPosition::ONE, SPIRE_VOXEL_FACE_POS_Z));
                vertices.push_back(PackVertexData(type, p.x + 0, p.y + 0, p.z + 1, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_POS_Z));
                break;

            case SPIRE_VOXEL_FACE_NEG_Z:
                vertices.push_back(PackVertexData(type, p.x + w, p.y + 0, p.z + 0, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_NEG_Z));
                vertices.push_back(PackVertexData(type, p.x + w, p.y + h, p.z + 0, VoxelVertexPosition::THREE, SPIRE_VOXEL_FACE_NEG_Z));
                vertices.push_back(PackVertexData(type, p.x + 0, p.y + h, p.z + 0, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_NEG_Z));
                vertices.push_back(PackVertexData(type, p.x + 0, p.y + h, p.z + 0, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_NEG_Z));
                vertices.push_back(PackVertexData(type, p.x + 0, p.y + 0, p.z + 0, VoxelVertexPosition::ONE, SPIRE_VOXEL_FACE_NEG_Z));
                vertices.push_back(PackVertexData(type, p.x + w, p.y + 0, p.z + 0, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_NEG_Z));
                break;

            case SPIRE_VOXEL_FACE_NEG_X:
                vertices.push_back(PackVertexData(type, p.x + 0, p.y + 0, p.z + 0, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_NEG_X));
                vertices.push_back(PackVertexData(type, p.x + 0, p.y + h, p.z + 0, VoxelVertexPosition::THREE, SPIRE_VOXEL_FACE_NEG_X));
                vertices.push_back(PackVertexData(type, p.x + 0, p.y + h, p.z + w, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_NEG_X));
                vertices.push_back(PackVertexData(type, p.x + 0, p.y + h, p.z + w, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_NEG_X));
                vertices.push_back(PackVertexData(type, p.x + 0, p.y + 0, p.z + w, VoxelVertexPosition::ONE, SPIRE_VOXEL_FACE_NEG_X));
                vertices.push_back(PackVertexData(type, p.x + 0, p.y + 0, p.z + 0, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_NEG_X));
                break;

            case SPIRE_VOXEL_FACE_POS_X:
                vertices.push_back(PackVertexData(type, p.x + 1, p.y + 0, p.z + w, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_POS_X));
                vertices.push_back(PackVertexData(type, p.x + 1, p.y + h, p.z + w, VoxelVertexPosition::THREE, SPIRE_VOXEL_FACE_POS_X));
                vertices.push_back(PackVertexData(type, p.x + 1, p.y + h, p.z + 0, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_POS_X));
                vertices.push_back(PackVertexData(type, p.x + 1, p.y + h, p.z + 0, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_POS_X));
                vertices.push_back(PackVertexData(type, p.x + 1, p.y + 0, p.z + 0, VoxelVertexPosition::ONE, SPIRE_VOXEL_FACE_POS_X));
                vertices.push_back(PackVertexData(type, p.x + 1, p.y + 0, p.z + w, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_POS_X));
                break;

            case SPIRE_VOXEL_FACE_POS_Y:
                vertices.push_back(PackVertexData(type, p.x + 0, p.y + 1, p.z + h, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_POS_Y));
                vertices.push_back(PackVertexData(type, p.x + 0, p.y + 1, p.z + 0, VoxelVertexPosition::THREE, SPIRE_VOXEL_FACE_POS_Y));
                vertices.push_back(PackVertexData(type, p.x + w, p.y + 1, p.z + 0, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_POS_Y));
                vertices.push_back(PackVertexData(type, p.x + w, p.y + 1, p.z + 0, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_POS_Y));
                vertices.push_back(PackVertexData(type, p.x + w, p.y + 1, p.z + h, VoxelVertexPosition::ONE, SPIRE_VOXEL_FACE_POS_Y));
                vertices.push_back(PackVertexData(type, p.x + 0, p.y + 1, p.z + h, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_POS_Y));
                break;

            case SPIRE_VOXEL_FACE_NEG_Y:
                vertices.push_back(PackVertexData(type, p.x + 0, p.y + 0, p.z + 0, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_NEG_Y));
                vertices.push_back(PackVertexData(type, p.x + 0, p.y + 0, p.z + h, VoxelVertexPosition::THREE, SPIRE_VOXEL_FACE_NEG_Y));
                vertices.push_back(PackVertexData(type, p.x + w, p.y + 0, p.z + h, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_NEG_Y));
                vertices.push_back(PackVertexData(type, p.x + w, p.y + 0, p.z + h, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_NEG_Y));
                vertices.push_back(PackVertexData(type, p.x + w, p.y + 0, p.z + 0, VoxelVertexPosition::ONE, SPIRE_VOXEL_FACE_NEG_Y));
                vertices.push_back(PackVertexData(type, p.x + 0, p.y + 0, p.z + 0, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_NEG_Y));
                break;
            default:
                assert(false);
                break;
        }
    }

    /*
u u u u
0 0 0 0
1 0 0 0
0 0 0 0
0 0 0 0
     */

    std::vector<VertexData> Chunk::GenerateMesh() const {
        std::vector<VertexData> vertices;
        const glm::u32 type = 1;

        std::array<GreedyMeshingBitmask, SPIRE_VOXEL_NUM_FACES * SPIRE_VOXEL_CHUNK_SIZE> masks;

        // slice, a, b are voxel chunk coordinates, but they could be different depending on face, see GreedyMeshingBitmask::GetVoxelIndex
        // slice is the slice of voxels we are working with
        // row is width, this is always the face axis (e.g. POS_Z/NEG_Z a is Z axis)
        // col is height, this is y unless the face axis is POS_Y/NEG_Y

        for (glm::u32 slice = 0; slice < SPIRE_VOXEL_CHUNK_SIZE; slice++) {
            for (glm::u32 row = 0; row < SPIRE_VOXEL_CHUNK_SIZE; row++) {
                for (glm::u32 col = 0; col < SPIRE_VOXEL_CHUNK_SIZE; col++) {
                    for (glm::u32 face = 0; face < SPIRE_VOXEL_NUM_FACES; face++) {
                        glm::uvec3 chunkCoords = GreedyMeshingBitmask::GetChunkCoords(slice, row, col, face);
                        if (VoxelData[SPIRE_VOXEL_POSITION_TO_INDEX(chunkCoords)] != VOXEL_TYPE_AIR && GetAdjacentVoxelType(*this, chunkCoords, face) == VOXEL_TYPE_AIR) {
                            masks[face * SPIRE_VOXEL_CHUNK_SIZE + slice].SetBit(row, col); // todo: go across chunk boundaries
                        }
                    }
                }
            }
        }

        for (glm::u32 face = 0; face < SPIRE_VOXEL_NUM_FACES; face++) {
            for (glm::u32 slice = 0; slice < SPIRE_VOXEL_CHUNK_SIZE; slice++) {
                // generate the bitmask
                GreedyMeshingBitmask &mask = masks[face * SPIRE_VOXEL_CHUNK_SIZE + slice];

                // push the faces
                for (glm::i32 col = 0; col < SPIRE_VOXEL_CHUNK_SIZE; col++) {
                    // find the starting row and height of the face
                    glm::u32 row = mask.NumTrailingEmptyVoxels(col, 0);
                    if (row >= SPIRE_VOXEL_CHUNK_SIZE) continue;
                    glm::u32 height = mask.NumTrailingPresentVoxels(col, row);

                    // absorb faces
                    mask.SetEmptyVoxels(col, row, height);

                    // move as far right as we can
                    glm::u32 width = 1;
                    while (col + width < SPIRE_VOXEL_CHUNK_SIZE && mask.NumTrailingPresentVoxels(col + width, row) >= height) {
                        mask.SetEmptyVoxels(col + width, row, height); // absorb the new column
                        width++;
                    }
                    //     mask.Print();

                    // push the face
                    glm::uvec3 chunkCoords = mask.GetChunkCoords(slice, row, col, face);
                    PushFace(vertices, type, face, chunkCoords, width, height);

                    if (mask.GetColumn(col) != 0) {
                        // we didn't get all the voxels on this row, loop again
                        col--;
                    }
                }
            }
        }


        // for
        // (size_t i =
        //          0;
        //  i < VoxelData.size();
        //  i
        //  ++
        // ) {
        //     if (VoxelData[i] == 0) continue;
        //
        //     glm::uvec3 p = SPIRE_VOXEL_INDEX_TO_POSITION(glm::vec3, i);
        //
        //     // Front (Z+)
        //     if (GetAdjacentVoxelType(*this, p, SPIRE_VOXEL_FACE_POS_Z) == VOXEL_TYPE_AIR) {
        //         PushFace(VoxelData, vertices, VoxelData[i],SPIRE_VOXEL_FACE_POS_Z, p);
        //     }
        //
        //     // Back (Z-)
        //     if (GetAdjacentVoxelType(*this, p, SPIRE_VOXEL_FACE_NEG_Z) == VOXEL_TYPE_AIR) {
        //         PushFace(VoxelData, vertices, VoxelData[i], SPIRE_VOXEL_FACE_NEG_Z, p);
        //     }
        //
        //     // Left (X-)
        //     if (GetAdjacentVoxelType(*this, p, SPIRE_VOXEL_FACE_NEG_X) == VOXEL_TYPE_AIR) {
        //         PushFace(VoxelData, vertices, VoxelData[i], SPIRE_VOXEL_FACE_NEG_X, p);
        //     }
        //
        //     // Right (X+)
        //     if (GetAdjacentVoxelType(*this, p, SPIRE_VOXEL_FACE_POS_X) == VOXEL_TYPE_AIR) {
        //         PushFace(VoxelData, vertices, VoxelData[i], SPIRE_VOXEL_FACE_POS_X, p);
        //     }
        //
        //     // Top (Y+)
        //     if (GetAdjacentVoxelType(*this, p, SPIRE_VOXEL_FACE_POS_Y) == VOXEL_TYPE_AIR) {
        //         PushFace(VoxelData, vertices, VoxelData[i], SPIRE_VOXEL_FACE_POS_Y, p);
        //     }
        //
        //     // Bottom (Y-)
        //     if (GetAdjacentVoxelType(*this, p, SPIRE_VOXEL_FACE_NEG_Y) == VOXEL_TYPE_AIR) {
        //         PushFace(VoxelData, vertices, VoxelData[i], SPIRE_VOXEL_FACE_NEG_Y, p);
        //     }
        // }

        return vertices;
    }

    ChunkData Chunk::GenerateChunkData(glm::u32 chunkIndex) const {
        assert(NumVertices > 0);
        return {
            .CPU_DrawCommandParams = {
                .vertexCount = (NumVertices),
                .instanceCount = 1,
                .firstVertex = static_cast<glm::u32>(Allocation.Start / sizeof(VertexData)),
                .firstInstance = chunkIndex
            },
            .ChunkX = ChunkPosition.x,
            .ChunkY = ChunkPosition.y,
            .ChunkZ = ChunkPosition.z
        };
    }

    std::optional<std::size_t> Chunk::GetIndexOfVoxel(glm::ivec3 chunkPosition, glm::ivec3 voxelWorldPosition) {
        if (chunkPosition != VoxelWorld::GetChunkPositionOfVoxel(voxelWorldPosition)) return std::nullopt;
        glm::uvec3 pos = voxelWorldPosition - chunkPosition * SPIRE_VOXEL_CHUNK_SIZE;
        return {SPIRE_VOXEL_POSITION_TO_INDEX(pos)};
    }
} // SpireVoxel
