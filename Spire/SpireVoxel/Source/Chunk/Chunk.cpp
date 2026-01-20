#include "Chunk.h"

#include "meshing/GreedyMeshingGrid.h"
#include "VoxelWorld.h"
#include "meshing/ChunkMesh.h"

namespace SpireVoxel {
    VoxelType GetAdjacentVoxelType(const Chunk &chunk, glm::ivec3 position, glm::u32 face) {
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

    void PushFace(std::vector<VertexData> &vertices, glm::u32 face, glm::uvec3 p, glm::u32 width, glm::u32 height) {
        assert(width > 0);
        assert(height > 0);
        const glm::u32 w = width;
        const glm::u32 h = height;
        switch (face) {
            case SPIRE_VOXEL_FACE_POS_Z:
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + 0, p.z + 1, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_POS_Z));
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + h, p.z + 1, VoxelVertexPosition::THREE, SPIRE_VOXEL_FACE_POS_Z));
                vertices.push_back(PackVertexData(w, h, p.x + w, p.y + h, p.z + 1, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_POS_Z));
                vertices.push_back(PackVertexData(w, h, p.x + w, p.y + h, p.z + 1, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_POS_Z));
                vertices.push_back(PackVertexData(w, h, p.x + w, p.y + 0, p.z + 1, VoxelVertexPosition::ONE, SPIRE_VOXEL_FACE_POS_Z));
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + 0, p.z + 1, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_POS_Z));
                break;

            case SPIRE_VOXEL_FACE_NEG_Z:
                vertices.push_back(PackVertexData(w, h, p.x + w, p.y + 0, p.z + 0, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_NEG_Z));
                vertices.push_back(PackVertexData(w, h, p.x + w, p.y + h, p.z + 0, VoxelVertexPosition::THREE, SPIRE_VOXEL_FACE_NEG_Z));
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + h, p.z + 0, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_NEG_Z));
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + h, p.z + 0, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_NEG_Z));
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + 0, p.z + 0, VoxelVertexPosition::ONE, SPIRE_VOXEL_FACE_NEG_Z));
                vertices.push_back(PackVertexData(w, h, p.x + w, p.y + 0, p.z + 0, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_NEG_Z));
                break;

            case SPIRE_VOXEL_FACE_NEG_X:
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + 0, p.z + 0, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_NEG_X));
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + h, p.z + 0, VoxelVertexPosition::THREE, SPIRE_VOXEL_FACE_NEG_X));
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + h, p.z + w, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_NEG_X));
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + h, p.z + w, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_NEG_X));
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + 0, p.z + w, VoxelVertexPosition::ONE, SPIRE_VOXEL_FACE_NEG_X));
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + 0, p.z + 0, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_NEG_X));
                break;

            case SPIRE_VOXEL_FACE_POS_X:
                vertices.push_back(PackVertexData(w, h, p.x + 1, p.y + 0, p.z + w, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_POS_X));
                vertices.push_back(PackVertexData(w, h, p.x + 1, p.y + h, p.z + w, VoxelVertexPosition::THREE, SPIRE_VOXEL_FACE_POS_X));
                vertices.push_back(PackVertexData(w, h, p.x + 1, p.y + h, p.z + 0, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_POS_X));
                vertices.push_back(PackVertexData(w, h, p.x + 1, p.y + h, p.z + 0, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_POS_X));
                vertices.push_back(PackVertexData(w, h, p.x + 1, p.y + 0, p.z + 0, VoxelVertexPosition::ONE, SPIRE_VOXEL_FACE_POS_X));
                vertices.push_back(PackVertexData(w, h, p.x + 1, p.y + 0, p.z + w, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_POS_X));
                break;

            case SPIRE_VOXEL_FACE_POS_Y:
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + 1, p.z + h, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_POS_Y));
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + 1, p.z + 0, VoxelVertexPosition::THREE, SPIRE_VOXEL_FACE_POS_Y));
                vertices.push_back(PackVertexData(w, h, p.x + w, p.y + 1, p.z + 0, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_POS_Y));
                vertices.push_back(PackVertexData(w, h, p.x + w, p.y + 1, p.z + 0, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_POS_Y));
                vertices.push_back(PackVertexData(w, h, p.x + w, p.y + 1, p.z + h, VoxelVertexPosition::ONE, SPIRE_VOXEL_FACE_POS_Y));
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + 1, p.z + h, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_POS_Y));
                break;

            case SPIRE_VOXEL_FACE_NEG_Y:
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + 0, p.z + 0, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_NEG_Y));
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + 0, p.z + h, VoxelVertexPosition::THREE, SPIRE_VOXEL_FACE_NEG_Y));
                vertices.push_back(PackVertexData(w, h, p.x + w, p.y + 0, p.z + h, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_NEG_Y));
                vertices.push_back(PackVertexData(w, h, p.x + w, p.y + 0, p.z + h, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_NEG_Y));
                vertices.push_back(PackVertexData(w, h, p.x + w, p.y + 0, p.z + 0, VoxelVertexPosition::ONE, SPIRE_VOXEL_FACE_NEG_Y));
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + 0, p.z + 0, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_NEG_Y));
                break;
            default:
                assert(false);
                break;
        }
    }

    void Chunk::SetVoxel(glm::u32 index, VoxelType type) {
        VoxelData[index] = type;
        VoxelBits[index] = static_cast<bool>(type);
    }

    void Chunk::SetVoxels(glm::u32 startIndex, glm::u32 endIndex, VoxelType type) {
        std::fill(VoxelData.data() + startIndex,
                  VoxelData.data() + endIndex,
                  type);

        for (glm::u32 i = startIndex; i < endIndex; ++i) {
            VoxelBits[i] = static_cast<bool>(type);
        }
    }

    ChunkMesh Chunk::GenerateMesh() const {
        ChunkMesh mesh;

        // slice, row, col are voxel chunk coordinates, but they could be different depending on face, see GreedyMeshingBitmask::GetChunkCoords
        // slice is the slice of voxels we are working with
        // POS_Z/NEG_Z col is X axis, row is Y axis, slice is Z

        for (glm::u32 face = 0; face < SPIRE_VOXEL_NUM_FACES; face += 2) {
            for (glm::u32 slice = 0; slice < SPIRE_VOXEL_CHUNK_SIZE; slice++) {
                // generate the grid
                std::array<GreedyMeshingGrid, 2> grids;

                for (glm::u32 row = 0; row < SPIRE_VOXEL_CHUNK_SIZE; row++) {
                    for (glm::u32 col = 0; col < SPIRE_VOXEL_CHUNK_SIZE; col++) {
                        glm::ivec3 chunkCoords = GreedyMeshingGrid::GetChunkCoords(slice, row, col, face);
                        glm::ivec3 adjacentPositive = chunkCoords + FaceToDirection(face);
                        glm::ivec3 adjacentNegative = chunkCoords - FaceToDirection(face);
                        bool adjacentPositiveIsPresent = adjacentPositive.x < SPIRE_VOXEL_CHUNK_SIZE &&
                                                         adjacentPositive.y < SPIRE_VOXEL_CHUNK_SIZE &&
                                                         adjacentPositive.z < SPIRE_VOXEL_CHUNK_SIZE &&
                                                         VoxelBits[SPIRE_VOXEL_POSITION_TO_INDEX(adjacentPositive)];
                        bool adjacentNegativeIsPresent = adjacentNegative.x >= 0 &&
                                                         adjacentNegative.y >= 0 &&
                                                         adjacentNegative.z >= 0 &&
                                                         VoxelBits[SPIRE_VOXEL_POSITION_TO_INDEX(adjacentNegative)];
                        // This seems to be getting compiler optimised really heavily
                        // I tried adding another check and not generating any faces if the adjacent is outside of chunk bounds
                        // and it reduced generation time by 20%
                        if (VoxelBits[SPIRE_VOXEL_POSITION_TO_INDEX(chunkCoords)] && !adjacentPositiveIsPresent) {
                            grids[0].SetBit(row, col);
                        }

                        if (VoxelBits[SPIRE_VOXEL_POSITION_TO_INDEX(chunkCoords)] && !adjacentNegativeIsPresent) {
                            grids[1].SetBit(row, col);
                        }
                    }
                }

                // push the faces
                for (glm::u32 faceSignIndex = 0; faceSignIndex < 2; faceSignIndex++) {
                    GreedyMeshingGrid &grid = grids[faceSignIndex];
                    for (glm::i32 col = 0; col < SPIRE_VOXEL_CHUNK_SIZE; col++) {
                        // find the starting row and height of the face
                        if (grid.GetColumn(col) == 0) continue;
                        glm::u32 row = grid.NumTrailingEmptyVoxels(col, 0);
                        glm::u32 height = grid.NumTrailingPresentVoxels(col, row);

                        // absorb faces
                        grid.SetEmptyVoxels(col, row, height);

                        // move as far right as we can
                        glm::u32 width = 1;
                        while (col + width < SPIRE_VOXEL_CHUNK_SIZE && grid.NumTrailingPresentVoxels(col + width, row) >= height) {
                            grid.SetEmptyVoxels(col + width, row, height); // absorb the new column
                            width++;
                        }

                        // push the face
                        glm::uvec3 chunkCoords = GreedyMeshingGrid::GetChunkCoords(slice, row, col, face + faceSignIndex);
                        PushFace(mesh.Vertices, face + faceSignIndex, chunkCoords, width, height);

                        if (grid.GetColumn(col) != 0) {
                            // we didn't get all the voxels on this row, loop again
                            col--;
                        }
                    }
                }
            }
        }

        return mesh;
    }

    ChunkData Chunk::GenerateChunkData(glm::u32 chunkIndex) const {
        assert(NumVertices > 0);
        return {
            .CPU_DrawCommandParams = {
                .vertexCount = (NumVertices),
                .instanceCount = 1,
                .firstVertex = static_cast<glm::u32>(VertexAllocation.Location.Start / sizeof(VertexData)),
                .firstInstance = chunkIndex
            },
            .ChunkX = ChunkPosition.x,
            .ChunkY = ChunkPosition.y,
            .ChunkZ = ChunkPosition.z,
            .VoxelDataChunkIndex = static_cast<glm::u32>(VoxelDataAllocation.Location.Start / sizeof(GPUChunkVoxelData))
        };
    }

    void Chunk::RegenerateVoxelBits() {
        for (std::size_t i = 0; i < VoxelData.size(); i++) {
            VoxelBits[i] = static_cast<bool>(VoxelData[i]); // todo can this be done in a single write?
        }
    }

    std::optional<std::size_t> Chunk::GetIndexOfVoxel(glm::ivec3 chunkPosition, glm::ivec3 voxelWorldPosition) {
        if (chunkPosition != VoxelWorld::GetChunkPositionOfVoxel(voxelWorldPosition)) return std::nullopt;
        glm::uvec3 pos = voxelWorldPosition - chunkPosition * SPIRE_VOXEL_CHUNK_SIZE;
        return {SPIRE_VOXEL_POSITION_TO_INDEX(pos)};
    }
} // SpireVoxel
