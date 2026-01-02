#include "EngineIncludes.h"
#include "../Assets/Shaders/ShaderInfo.h"
#include <gtest/gtest.h>
#include "Chunk/GreedyMeshingGrid.h"

TEST(TestSettingBits, GreedyMeshingTests) {
    std::array<glm::u64, SPIRE_VOXEL_CHUNK_SIZE> bits{};
    SpireVoxel::GreedyMeshingGrid mask(bits.data());
    mask.SetBit(0, 0);

    EXPECT_EQ(mask.GetColumn(0), 1);

    mask.SetBit(2, 0);
    mask.SetBit(5, 0);

    EXPECT_EQ(mask.GetColumn(0), 0b100101);
}

TEST(TestGettingBits, GreedyMeshingTests) {
    std::array<glm::u64, SPIRE_VOXEL_CHUNK_SIZE> bits{};
    SpireVoxel::GreedyMeshingGrid mask(bits.data());

    EXPECT_FALSE(mask.GetBit(0, 0));
    EXPECT_FALSE(mask.GetBit(3, 0));
    EXPECT_FALSE(mask.GetBit(8, 19));

    mask.SetBit(0, 0);
    EXPECT_TRUE(mask.GetBit(0, 0));

    mask.SetBit(2, 0);
    EXPECT_TRUE(mask.GetBit(2, 0));
    EXPECT_FALSE(mask.GetBit(2, 1));
    EXPECT_FALSE(mask.GetBit(1, 2));
}

TEST(TestTrailingVoxelsA, GreedyMeshingTests) {
    std::array<glm::u64, SPIRE_VOXEL_CHUNK_SIZE> bits{};
    SpireVoxel::GreedyMeshingGrid mask(bits.data());
    EXPECT_EQ(mask.NumTrailingPresentVoxels(0, 0), 0);

    mask.SetBit(0, 0);
    mask.SetBit(2, 0);
    mask.SetBit(3, 0);
    mask.SetBit(5, 0);

    // 101101
    EXPECT_EQ(mask.NumTrailingEmptyVoxels(0, 0), 0);
    EXPECT_EQ(mask.NumTrailingPresentVoxels(0, 0), 1);
    EXPECT_EQ(mask.NumTrailingEmptyVoxels(0, 1), 1);
    EXPECT_EQ(mask.NumTrailingPresentVoxels(0, 2), 2);
}

TEST(TestTrailingVoxelsB, GreedyMeshingTests) {
    std::array<glm::u64, SPIRE_VOXEL_CHUNK_SIZE> bits{};
    SpireVoxel::GreedyMeshingGrid mask(bits.data());

    for (int i = 1; i < SPIRE_VOXEL_CHUNK_SIZE; i++) {
        mask.SetBit(i, 0);
    }

    EXPECT_EQ(mask.NumTrailingEmptyVoxels(0, 0), 1);
    EXPECT_EQ(mask.NumTrailingPresentVoxels(0, 0), 0);

    mask.SetBit(0, 0);

    EXPECT_EQ(mask.NumTrailingEmptyVoxels(0, 0), 0);
    EXPECT_EQ(mask.NumTrailingPresentVoxels(0, 0), SPIRE_VOXEL_CHUNK_SIZE);
}