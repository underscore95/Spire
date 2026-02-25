#include "Chunk.h"

#include "AOLookupTable.h"
#include "Meshing/GreedyMeshingGrid.h"
#include "VoxelWorld.h"
#include "Meshing/ChunkMesh.h"

namespace SpireVoxel {
    VoxelType GetAdjacentVoxelType(const Chunk &chunk, glm::ivec3 queryPosition) {
        glm::ivec3 queryChunkPosition = chunk.ChunkPosition;

        // Just for current usage but these asserts could be removed
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
            queryChunk = chunk.World.TryGetLoadedChunk(queryChunkPosition);
            if (!queryChunk) return VOXEL_TYPE_AIR;
        }
        return queryChunk->VoxelData[SPIRE_VOXEL_POSITION_TO_INDEX(queryPosition)];
    }


    void Chunk::PushRelatedFaceData(ChunkMesh &mesh, glm::uvec3 start, glm::u32 width, glm::u32 height, glm::u32 face) const {
        assert(start.x < SPIRE_VOXEL_CHUNK_SIZE);
        assert(start.y < SPIRE_VOXEL_CHUNK_SIZE);
        assert(start.z < SPIRE_VOXEL_CHUNK_SIZE);

        glm::uvec3 worldSize = GreedyMeshingFaceSizeSwizzleToWorldSpace(face, width, height);
        assert((worldSize.x == 0) + (worldSize.y == 0) + (worldSize.z == 0) == 1); // exactly one coordinate is 0

        if (IsFaceOnXAxis(face)) {
            assert(worldSize.x == 0);
            assert(worldSize.y == height);
            assert(worldSize.z == width);
            constexpr glm::u32 xOffset = 0;
            // must be height in outer, width in inner for all 3 faces
            for (glm::u32 yOffset = 0; yOffset < worldSize.y; yOffset++) {
                for (glm::u32 zOffset = 0; zOffset < worldSize.z; zOffset++) {
                    glm::uvec3 coord = start + glm::uvec3(xOffset, yOffset, zOffset);
                    PushRelatedVoxelData(mesh, coord, face);
                }
            }
        } else if (IsFaceOnYAxis(face)) {
            assert(worldSize.y == 0);
            assert(worldSize.x == width);
            assert(worldSize.z == height);
            constexpr glm::u32 yOffset = 0;
            for (glm::u32 zOffset = 0; zOffset < worldSize.z; zOffset++) {
                for (glm::u32 xOffset = 0; xOffset < worldSize.x; xOffset++) {
                    glm::uvec3 coord = start + glm::uvec3(xOffset, yOffset, zOffset);
                    PushRelatedVoxelData(mesh, coord, face);
                }
            }
        } else if (IsFaceOnZAxis(face)) {
            assert(worldSize.z == 0);
            assert(worldSize.y == height);
            assert(worldSize.x == width);
            constexpr glm::u32 zOffset = 0;
            for (glm::u32 yOffset = 0; yOffset < worldSize.y; yOffset++) {
                for (glm::u32 xOffset = 0; xOffset < worldSize.x; xOffset++) {
                    glm::uvec3 coord = start + glm::uvec3(xOffset, yOffset, zOffset);
                    PushRelatedVoxelData(mesh, coord, face);
                }
            }
        } else {
            assert(false);
        }
    }

    // https://0fps.net/2013/07/03/ambient-occlusion-for-minecraft-like-worlds/
    glm::u32 GetVertexAO(bool side1, bool side2, bool corner) {
        if (side1 && side2) {
            return 3;
        }
        return (side1 + side2 + corner);
    }

    void Chunk::PushRelatedVoxelData(ChunkMesh &mesh, glm::uvec3 chunkCoords, glm::u32 face) const {
        assert(chunkCoords.x < SPIRE_VOXEL_CHUNK_SIZE);
        assert(chunkCoords.y < SPIRE_VOXEL_CHUNK_SIZE);
        assert(chunkCoords.z < SPIRE_VOXEL_CHUNK_SIZE);

        VoxelType type = VoxelData[SPIRE_VOXEL_POSITION_TO_INDEX(chunkCoords)];
        assert(type != 0);

        // Push voxel type
        mesh.VoxelTypes.push_back(type);

        // Push AO
        glm::ivec3 i, j, k;
        for (VoxelVertexPosition vertexPos : VoxelVertexPositionValues) {
            glm::u32 aoIndex = mesh.AODataValueCount % SPIRE_AO_VALUES_PER_U32;

            GetAmbientOcclusionOffsetVectors(face, static_cast<glm::u32>(vertexPos), i, j, k);

            bool side1 = GetAdjacentVoxelType(*this, glm::ivec3(chunkCoords) + i + j) != VOXEL_TYPE_AIR;
            bool side2 = GetAdjacentVoxelType(*this, glm::ivec3(chunkCoords) + i + k) != VOXEL_TYPE_AIR;
            bool corner = GetAdjacentVoxelType(*this, glm::ivec3(chunkCoords) + i + j + k) != VOXEL_TYPE_AIR;
            glm::u32 ao = GetVertexAO(side1, side2, corner);
            assert(ao <= 0b11);

            if (aoIndex == 0) {
                // need a new integer
                mesh.AOData.push_back(ao);
            } else {
                // there is space to pack it into the end of the current integer
                glm::u32 &packed = mesh.AOData.back();
                packed = SetAO(packed, aoIndex, ao);
            }
            mesh.AODataValueCount++;
        }
    }

    void Chunk::PushFace(ChunkMesh &mesh, glm::u32 face, glm::uvec3 p, glm::u32 width, glm::u32 height) const {
        std::vector<VertexData> &vertices = mesh.Vertices;

        assert(width > 0);
        assert(height > 0);
        const glm::u32 w = width;
        const glm::u32 h = height;
        switch (face) {
            case SPIRE_VOXEL_FACE_POS_Z:
                vertices.push_back(PackVertexData(mesh.VoxelTypes.size(), w, h, p.x + 0, p.y + 0, p.z + 1, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_POS_Z));
                vertices.push_back(PackVertexData(mesh.VoxelTypes.size(), w, h, p.x + 0, p.y + h, p.z + 1, VoxelVertexPosition::THREE, SPIRE_VOXEL_FACE_POS_Z));
                vertices.push_back(PackVertexData(mesh.VoxelTypes.size(), w, h, p.x + w, p.y + h, p.z + 1, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_POS_Z));
                vertices.push_back(PackVertexData(mesh.VoxelTypes.size(), w, h, p.x + w, p.y + h, p.z + 1, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_POS_Z));
                vertices.push_back(PackVertexData(mesh.VoxelTypes.size(), w, h, p.x + w, p.y + 0, p.z + 1, VoxelVertexPosition::ONE, SPIRE_VOXEL_FACE_POS_Z));
                vertices.push_back(PackVertexData(mesh.VoxelTypes.size(), w, h, p.x + 0, p.y + 0, p.z + 1, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_POS_Z));
                break;

            case SPIRE_VOXEL_FACE_NEG_Z:
                vertices.push_back(PackVertexData(mesh.VoxelTypes.size(), w, h, p.x + w, p.y + 0, p.z + 0, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_NEG_Z));
                vertices.push_back(PackVertexData(mesh.VoxelTypes.size(), w, h, p.x + w, p.y + h, p.z + 0, VoxelVertexPosition::THREE, SPIRE_VOXEL_FACE_NEG_Z));
                vertices.push_back(PackVertexData(mesh.VoxelTypes.size(), w, h, p.x + 0, p.y + h, p.z + 0, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_NEG_Z));
                vertices.push_back(PackVertexData(mesh.VoxelTypes.size(), w, h, p.x + 0, p.y + h, p.z + 0, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_NEG_Z));
                vertices.push_back(PackVertexData(mesh.VoxelTypes.size(), w, h, p.x + 0, p.y + 0, p.z + 0, VoxelVertexPosition::ONE, SPIRE_VOXEL_FACE_NEG_Z));
                vertices.push_back(PackVertexData(mesh.VoxelTypes.size(), w, h, p.x + w, p.y + 0, p.z + 0, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_NEG_Z));
                break;

            case SPIRE_VOXEL_FACE_NEG_X:
                vertices.push_back(PackVertexData(mesh.VoxelTypes.size(), w, h, p.x + 0, p.y + 0, p.z + 0, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_NEG_X));
                vertices.push_back(PackVertexData(mesh.VoxelTypes.size(), w, h, p.x + 0, p.y + h, p.z + 0, VoxelVertexPosition::THREE, SPIRE_VOXEL_FACE_NEG_X));
                vertices.push_back(PackVertexData(mesh.VoxelTypes.size(), w, h, p.x + 0, p.y + h, p.z + w, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_NEG_X));
                vertices.push_back(PackVertexData(mesh.VoxelTypes.size(), w, h, p.x + 0, p.y + h, p.z + w, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_NEG_X));
                vertices.push_back(PackVertexData(mesh.VoxelTypes.size(), w, h, p.x + 0, p.y + 0, p.z + w, VoxelVertexPosition::ONE, SPIRE_VOXEL_FACE_NEG_X));
                vertices.push_back(PackVertexData(mesh.VoxelTypes.size(), w, h, p.x + 0, p.y + 0, p.z + 0, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_NEG_X));
                break;

            case SPIRE_VOXEL_FACE_POS_X:
                vertices.push_back(PackVertexData(mesh.VoxelTypes.size(), w, h, p.x + 1, p.y + 0, p.z + w, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_POS_X));
                vertices.push_back(PackVertexData(mesh.VoxelTypes.size(), w, h, p.x + 1, p.y + h, p.z + w, VoxelVertexPosition::THREE, SPIRE_VOXEL_FACE_POS_X));
                vertices.push_back(PackVertexData(mesh.VoxelTypes.size(), w, h, p.x + 1, p.y + h, p.z + 0, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_POS_X));
                vertices.push_back(PackVertexData(mesh.VoxelTypes.size(), w, h, p.x + 1, p.y + h, p.z + 0, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_POS_X));
                vertices.push_back(PackVertexData(mesh.VoxelTypes.size(), w, h, p.x + 1, p.y + 0, p.z + 0, VoxelVertexPosition::ONE, SPIRE_VOXEL_FACE_POS_X));
                vertices.push_back(PackVertexData(mesh.VoxelTypes.size(), w, h, p.x + 1, p.y + 0, p.z + w, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_POS_X));
                break;

            case SPIRE_VOXEL_FACE_POS_Y:
                vertices.push_back(PackVertexData(mesh.VoxelTypes.size(), w, h, p.x + 0, p.y + 1, p.z + h, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_POS_Y));
                vertices.push_back(PackVertexData(mesh.VoxelTypes.size(), w, h, p.x + 0, p.y + 1, p.z + 0, VoxelVertexPosition::THREE, SPIRE_VOXEL_FACE_POS_Y));
                vertices.push_back(PackVertexData(mesh.VoxelTypes.size(), w, h, p.x + w, p.y + 1, p.z + 0, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_POS_Y));
                vertices.push_back(PackVertexData(mesh.VoxelTypes.size(), w, h, p.x + w, p.y + 1, p.z + 0, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_POS_Y));
                vertices.push_back(PackVertexData(mesh.VoxelTypes.size(), w, h, p.x + w, p.y + 1, p.z + h, VoxelVertexPosition::ONE, SPIRE_VOXEL_FACE_POS_Y));
                vertices.push_back(PackVertexData(mesh.VoxelTypes.size(), w, h, p.x + 0, p.y + 1, p.z + h, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_POS_Y));
                break;

            case SPIRE_VOXEL_FACE_NEG_Y:
                vertices.push_back(PackVertexData(mesh.VoxelTypes.size(), w, h, p.x + 0, p.y + 0, p.z + 0, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_NEG_Y));
                vertices.push_back(PackVertexData(mesh.VoxelTypes.size(), w, h, p.x + 0, p.y + 0, p.z + h, VoxelVertexPosition::THREE, SPIRE_VOXEL_FACE_NEG_Y));
                vertices.push_back(PackVertexData(mesh.VoxelTypes.size(), w, h, p.x + w, p.y + 0, p.z + h, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_NEG_Y));
                vertices.push_back(PackVertexData(mesh.VoxelTypes.size(), w, h, p.x + w, p.y + 0, p.z + h, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_NEG_Y));
                vertices.push_back(PackVertexData(mesh.VoxelTypes.size(), w, h, p.x + w, p.y + 0, p.z + 0, VoxelVertexPosition::ONE, SPIRE_VOXEL_FACE_NEG_Y));
                vertices.push_back(PackVertexData(mesh.VoxelTypes.size(), w, h, p.x + 0, p.y + 0, p.z + 0, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_NEG_Y));
                break;
            default:
                assert(false);
                break;
        }

        PushRelatedFaceData(mesh, p, width, height, face);
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

    static std::string VertexDataToString(const VertexData &v) {
        const glm::u32 width = (v.Packed_6Width6Height & 0x3Fu) + 1u;
        const glm::u32 height = ((v.Packed_6Width6Height >> 6) & 0x3Fu) + 1u;

        const glm::u32 z = (v.Packed_7X7Y7Z2VertPos3Face & 0x7Fu);
        const glm::u32 y = ((v.Packed_7X7Y7Z2VertPos3Face >> 7) & 0x7Fu);
        const glm::u32 x = ((v.Packed_7X7Y7Z2VertPos3Face >> 14) & 0x7Fu);

        const glm::u32 vertPos = ((v.Packed_7X7Y7Z2VertPos3Face >> 21) & 0x3u);
        const glm::u32 face = ((v.Packed_7X7Y7Z2VertPos3Face >> 23) & 0x7u);

        std::ostringstream ss;
        ss << "VertexData {\n"
                << "  Width: " << width << ", Height: " << height << "\n"
                << "  ChunkPos: (X: " << x << ", Y: " << y << ", Z: " << z << ")\n"
                << "  VertexPosition: " << vertPos << "\n"
                << "  Face: " << face << "\n"
                << "  VoxelTypeStartingIndex: " << v.VoxelTypeStartingIndex << "\n"
                << "}";

        return ss.str();
    }

    ChunkMesh Chunk::GenerateMesh() const {
        Spire::Timer timer;
        ChunkMesh mesh = {};

        // slice, row, col are voxel chunk coordinates, but they could be different depending on face, see GreedyMeshingBitmask::GetChunkCoords
        // slice is the slice of voxels we are working with
        // POS_Z/NEG_Z col is X axis, row is Y axis, slice is Z

        std::array<GreedyMeshingGrid, SPIRE_VOXEL_CHUNK_SIZE * SPIRE_VOXEL_NUM_FACES> grids = {};

        // generate the grids
        for (glm::u32 slice = 0; slice < SPIRE_VOXEL_CHUNK_SIZE; slice++) {
            for (glm::u32 row = 0; row < SPIRE_VOXEL_CHUNK_SIZE; row++) {
                for (glm::u32 col = 0; col < SPIRE_VOXEL_CHUNK_SIZE; col++) {
                    for (glm::u32 face = 0; face < SPIRE_VOXEL_NUM_FACES; face++) {
                        glm::ivec3 chunkCoords = GreedyMeshingGrid::GetChunkCoords(slice, row, col, face);
                        glm::ivec3 adjacent = chunkCoords + FaceToDirection(face);
                        bool adjacentIsPresent = adjacent.x < SPIRE_VOXEL_CHUNK_SIZE && adjacent.x >= 0 &&
                                                 adjacent.y < SPIRE_VOXEL_CHUNK_SIZE && adjacent.y >= 0 &&
                                                 adjacent.z < SPIRE_VOXEL_CHUNK_SIZE && adjacent.z >= 0 &&
                                                 VoxelBits[SPIRE_VOXEL_POSITION_TO_INDEX(adjacent)];

                        if (VoxelBits[SPIRE_VOXEL_POSITION_TO_INDEX(chunkCoords)] && !adjacentIsPresent) {
                            grids[slice * SPIRE_VOXEL_NUM_FACES + face].SetBit(row, col);
                        }
                    }
                }
            }
        }

        for (glm::u32 face = 0; face < SPIRE_VOXEL_NUM_FACES; face++) {
            for (glm::u32 slice = 0; slice < SPIRE_VOXEL_CHUNK_SIZE; slice++) {
                // push the faces
                GreedyMeshingGrid &grid = grids[slice * SPIRE_VOXEL_NUM_FACES + face];
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
                    glm::uvec3 chunkCoords = GreedyMeshingGrid::GetChunkCoords(slice, row, col, face);
                    PushFace(mesh, face, chunkCoords, width, height);

                    if (grid.GetColumn(col) != 0) {
                        // we didn't get all the voxels on this row, loop again
                        col--;
                    }
                }
            }
        }
        float millis = timer.MillisSinceStart();
        Spire::info("generated in {} ms", millis);

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
            .VoxelDataChunkIndex = static_cast<glm::u32>(VoxelDataAllocation.Location.Start / sizeof(VoxelType)),
            .VoxelDataAllocationIndex = static_cast<glm::u32>(VoxelDataAllocation.Location.AllocationIndex),
            .VertexBufferIndex = static_cast<glm::u32>(VertexAllocation.Location.AllocationIndex),
            .LODScale = static_cast<float>(LOD.Scale),
            .AODataChunkPackedIndex = static_cast<glm::u32>(AODataAllocation.Location.Start / sizeof(glm::u32)),
            .AODataAllocationIndex = static_cast<glm::u32>(AODataAllocation.Location.AllocationIndex)
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
