#include "EngineIncludes.h"
#include "../Assets/Shaders/ShaderInfo.h"
#include <gtest/gtest.h>

TEST(VoxelUnpackTests, UnpackFromZero) {
    glm::u32 packed = 0;

    EXPECT_EQ(SPIRE_VOXEL_UNPACK_VOXEL_TYPE(packed, 0), 0u);
    EXPECT_EQ(SPIRE_VOXEL_UNPACK_VOXEL_TYPE(packed, 1), 0u);
}

TEST(VoxelUnpackTests, UnpackOnlyEvenSet) {
    glm::u32 packed = 0;
    packed = SPIRE_VOXEL_PACK_VOXEL_TYPE(packed, 0, 0x89AF);

    EXPECT_EQ(SPIRE_VOXEL_UNPACK_VOXEL_TYPE(packed, 0), 0x89AF);
    EXPECT_EQ(SPIRE_VOXEL_UNPACK_VOXEL_TYPE(packed, 1), 0u);
}

TEST(VoxelUnpackTests, UnpackOnlyOddSet) {
    glm::u32 packed = 0;
    packed = SPIRE_VOXEL_PACK_VOXEL_TYPE(packed, 1, 0xB31F);

    EXPECT_EQ(SPIRE_VOXEL_UNPACK_VOXEL_TYPE(packed, 0), 0u);
    EXPECT_EQ(SPIRE_VOXEL_UNPACK_VOXEL_TYPE(packed, 1), 0xB31F);
}

TEST(VoxelUnpackTests, UnpackMaxValues) {
    glm::u32 packed = 0xFFFFFFFFu;

    EXPECT_EQ(SPIRE_VOXEL_UNPACK_VOXEL_TYPE(packed, 0), 0xFFFFu);
    EXPECT_EQ(SPIRE_VOXEL_UNPACK_VOXEL_TYPE(packed, 1), 0xFFFFu);
}

TEST(VoxelUnpackTests, UnpackLargeIndex) {
    glm::u32 packed = 0;
    packed = SPIRE_VOXEL_PACK_VOXEL_TYPE(packed, 0, 0x1234);
    packed = SPIRE_VOXEL_PACK_VOXEL_TYPE(packed, 1, 0x5678);

    EXPECT_EQ(SPIRE_VOXEL_UNPACK_VOXEL_TYPE(packed, 2), 0x1234);
    EXPECT_EQ(SPIRE_VOXEL_UNPACK_VOXEL_TYPE(packed, 3), 0x5678);
}

TEST(VoxelPackTests, UnpackEvenOdd) {
    glm::u32 packed = 0;
    packed = SPIRE_VOXEL_PACK_VOXEL_TYPE(packed, 0, 0x1234);
    packed = SPIRE_VOXEL_PACK_VOXEL_TYPE(packed, 1, 0xABCD);

    EXPECT_EQ(SPIRE_VOXEL_UNPACK_VOXEL_TYPE(packed, 0), 0x1234);
    EXPECT_EQ(SPIRE_VOXEL_UNPACK_VOXEL_TYPE(packed, 1), 0xABCD);
}

TEST(VoxelPackTests, OverwriteEvenOnly) {
    glm::u32 packed = 0;
    packed = SPIRE_VOXEL_PACK_VOXEL_TYPE(packed, 0, 0x1111);
    packed = SPIRE_VOXEL_PACK_VOXEL_TYPE(packed, 1, 0x2222);
    packed = SPIRE_VOXEL_PACK_VOXEL_TYPE(packed, 0, 0x3333);

    EXPECT_EQ(SPIRE_VOXEL_UNPACK_VOXEL_TYPE(packed, 0), 0x3333);
    EXPECT_EQ(SPIRE_VOXEL_UNPACK_VOXEL_TYPE(packed, 1), 0x2222);
}

TEST(VoxelPackTests, OverwriteOddOnly) {
    glm::u32 packed = 0;
    packed = SPIRE_VOXEL_PACK_VOXEL_TYPE(packed, 0, 0xAAAA);
    packed = SPIRE_VOXEL_PACK_VOXEL_TYPE(packed, 1, 0xBBBB);
    packed = SPIRE_VOXEL_PACK_VOXEL_TYPE(packed, 1, 0xCCCC);

    EXPECT_EQ(SPIRE_VOXEL_UNPACK_VOXEL_TYPE(packed, 0), 0xAAAA);
    EXPECT_EQ(SPIRE_VOXEL_UNPACK_VOXEL_TYPE(packed, 1), 0xCCCC);
}

TEST(VoxelPackTests, MaskingToUint16) {
    glm::u32 packed = 0;
    packed = SPIRE_VOXEL_PACK_VOXEL_TYPE(packed, 1, 0x2FFFF);
    packed = SPIRE_VOXEL_PACK_VOXEL_TYPE(packed, 0, 0x1FFFF);

    EXPECT_EQ(SPIRE_VOXEL_UNPACK_VOXEL_TYPE(packed, 0), 0xFFFF);
    EXPECT_EQ(SPIRE_VOXEL_UNPACK_VOXEL_TYPE(packed, 1), 0xFFFF);
}

TEST(VoxelPackTests, TestPack_1_1) {
    glm::u32 packed = 0;
    packed = SPIRE_VOXEL_PACK_VOXEL_TYPE(packed, 0, 1);
    packed = SPIRE_VOXEL_PACK_VOXEL_TYPE(packed, 1, 1);

    EXPECT_EQ(SPIRE_VOXEL_UNPACK_VOXEL_TYPE(packed, 0), 1);
    EXPECT_EQ(SPIRE_VOXEL_UNPACK_VOXEL_TYPE(packed, 1), 1);
}

TEST(VoxelPackTests, TestPack_ChunkMax) {
    glm::u32 packed = 0;
    packed = SPIRE_VOXEL_PACK_VOXEL_TYPE(packed, SPIRE_VOXEL_CHUNK_VOLUME - 1, 0xF83F);
    packed = SPIRE_VOXEL_PACK_VOXEL_TYPE(packed, SPIRE_VOXEL_CHUNK_VOLUME - 2, 0x0130);

    EXPECT_EQ(SPIRE_VOXEL_UNPACK_VOXEL_TYPE(packed, SPIRE_VOXEL_CHUNK_VOLUME - 1), 0xF83F);
    EXPECT_EQ(SPIRE_VOXEL_UNPACK_VOXEL_TYPE(packed, SPIRE_VOXEL_CHUNK_VOLUME - 2), 0x0130);
}