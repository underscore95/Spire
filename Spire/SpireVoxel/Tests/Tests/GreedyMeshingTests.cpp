#include "EngineIncludes.h"
#include "../Assets/Shaders/ShaderInfo.h"
#include <gtest/gtest.h>
#include "TestHelpers.h"
#include "../../Source/Chunk/Meshing/GreedyMeshingGrid.h"

TEST(GreedyMeshingTests, TestSettingBits) {
    SpireVoxel::GreedyMeshingGrid mask;
    mask.SetBit(0, 0);

    EXPECT_EQ(mask.GetColumn(0), 1);

    mask.SetBit(2, 0);
    mask.SetBit(5, 0);

    EXPECT_EQ(mask.GetColumn(0), 0b100101);
}

TEST(GreedyMeshingTests, TestGettingBits) {
    SpireVoxel::GreedyMeshingGrid mask;
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

TEST(GreedyMeshingTests, TestTrailingVoxelsA) {
    SpireVoxel::GreedyMeshingGrid mask;
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

TEST(GreedyMeshingTests, TestTrailingVoxelsB) {
    SpireVoxel::GreedyMeshingGrid mask;

    for (int i = 1; i < SPIRE_VOXEL_CHUNK_SIZE; i++) {
        mask.SetBit(i, 0);
    }

    EXPECT_EQ(mask.NumTrailingEmptyVoxels(0, 0), 1);
    EXPECT_EQ(mask.NumTrailingPresentVoxels(0, 0), 0);

    mask.SetBit(0, 0);

    EXPECT_EQ(mask.NumTrailingEmptyVoxels(0, 0), 0);
    EXPECT_EQ(mask.NumTrailingPresentVoxels(0, 0), SPIRE_VOXEL_CHUNK_SIZE);
}
