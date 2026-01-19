#include "EngineIncludes.h"
#include "../Assets/Shaders/ShaderInfo.h"
#include <gtest/gtest.h>

#include "TestHelpers.h"
#include "Edits/CuboidVoxelEdit.h"

TEST(VoxelEditTests, TestCuboidEditsA) {
    // single chunk, fully inside, half-open range
    std::vector edits = SpireVoxel::CuboidVoxelEdit::GenerateEdits({0, 0, 0}, {3, 5, 2});
    EXPECT_EQ(edits.size(), 1);

    EXPECT_IVEC3_EQ(edits[0].ChunkPosition, glm::ivec3(0, 0, 0));
    EXPECT_UVEC3_EQ(edits[0].RectOrigin, glm::uvec3(0, 0, 0));
    EXPECT_UVEC3_EQ(edits[0].RectSize,  glm::uvec3(2, 4, 1));
}

TEST(VoxelEditTests, TestCuboidEditsB) {
    // crosses negative X boundary into chunk (0,0,0)
    std::vector edits = SpireVoxel::CuboidVoxelEdit::GenerateEdits({-1, 0, 0}, {3, 5, 2});
    EXPECT_EQ(edits.size(), 2);

    EXPECT_IVEC3_EQ(edits[0].ChunkPosition, glm::ivec3(-1, 0, 0));
    EXPECT_UVEC3_EQ(edits[0].RectOrigin,
                    glm::uvec3(SPIRE_VOXEL_CHUNK_SIZE - 1, 0, 0));
    EXPECT_UVEC3_EQ(edits[0].RectSize, glm::uvec3(1, 4, 1));

    EXPECT_IVEC3_EQ(edits[1].ChunkPosition, glm::ivec3(0, 0, 0));
    EXPECT_UVEC3_EQ(edits[1].RectOrigin, glm::uvec3(0, 0, 0));
    EXPECT_UVEC3_EQ(edits[1].RectSize,  glm::uvec3(1, 4, 1));
}

TEST(VoxelEditTests, TestCuboidEditsC) {
    // crosses negative X and Y boundaries
    std::vector edits = SpireVoxel::CuboidVoxelEdit::GenerateEdits({-1, -1, 0}, {3, 5, 6});
    EXPECT_EQ(edits.size(), 4);

    EXPECT_IVEC3_EQ(edits[0].ChunkPosition, glm::ivec3(-1, -1, 0));
    EXPECT_UVEC3_EQ(edits[0].RectOrigin,
                    glm::uvec3(SPIRE_VOXEL_CHUNK_SIZE - 1,
                               SPIRE_VOXEL_CHUNK_SIZE - 1,
                               0));
    EXPECT_UVEC3_EQ(edits[0].RectSize, glm::uvec3(1, 1, 5));

    EXPECT_IVEC3_EQ(edits[1].ChunkPosition, glm::ivec3(-1, 0, 0));
    EXPECT_UVEC3_EQ(edits[1].RectOrigin,
                    glm::uvec3(SPIRE_VOXEL_CHUNK_SIZE - 1, 0, 0));
    EXPECT_UVEC3_EQ(edits[1].RectSize, glm::uvec3(1, 3, 5));

    EXPECT_IVEC3_EQ(edits[2].ChunkPosition, glm::ivec3(0, -1, 0));
    EXPECT_UVEC3_EQ(edits[2].RectOrigin,
                    glm::uvec3(0, SPIRE_VOXEL_CHUNK_SIZE - 1, 0));
    EXPECT_UVEC3_EQ(edits[2].RectSize, glm::uvec3(1, 1, 5));

    EXPECT_IVEC3_EQ(edits[3].ChunkPosition, glm::ivec3(0, 0, 0));
    EXPECT_UVEC3_EQ(edits[3].RectOrigin, glm::uvec3(0, 0, 0));
    EXPECT_UVEC3_EQ(edits[3].RectSize,  glm::uvec3(1, 3, 5));
}

TEST(VoxelEditTests, TestCalculateAffectedChunkMeshesA) {
    // edit touches negative X, positive X, negative Y and negative Z boundaries
    std::vector<SpireVoxel::CuboidVoxelEdit::Edit> edits = {
        {{0, 0, 0}, {0, 0, 0}, {SPIRE_VOXEL_CHUNK_SIZE, 1, 5}}
    };

    std::unordered_set chunks =
        SpireVoxel::CuboidVoxelEdit::CalculateAffectedChunkMeshes(edits);

    EXPECT_EQ(chunks.size(), 5);

    EXPECT_TRUE(chunks.contains(glm::ivec3(0, 0, 0)));
    EXPECT_TRUE(chunks.contains(glm::ivec3(-1, 0, 0)));
    EXPECT_TRUE(chunks.contains(glm::ivec3(1, 0, 0)));
    EXPECT_TRUE(chunks.contains(glm::ivec3(0, -1, 0)));
    EXPECT_TRUE(chunks.contains(glm::ivec3(0, 0, -1)));
}
