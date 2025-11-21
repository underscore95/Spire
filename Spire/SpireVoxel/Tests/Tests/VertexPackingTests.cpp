#include "EngineIncludes.h"
#include "../Assets/Shaders/ShaderInfo.h"
#include <gtest/gtest.h>

TEST(TestA, VertexPackingTests) {
    SpireVoxel::VertexData vertex = SpireVoxel::PackVertexData(0, 14, 50, 15, SpireVoxel::VoxelVertexPosition::ZERO, 3);
    glm::uvec3 pos = SpireVoxel::UnpackVertexDataXYZ(vertex.Packed_8X8Y8Z2VertPos3Face);
    EXPECT_EQ(pos.x, 14);
    EXPECT_EQ(pos.y, 50);
    EXPECT_EQ(pos.z, 15);
    EXPECT_EQ(SpireVoxel::UnpackVertexDataVertexPosition(vertex.Packed_8X8Y8Z2VertPos3Face), SpireVoxel::VoxelVertexPosition::ZERO);
    EXPECT_EQ(SpireVoxel::UnpackVertexDataFace(vertex.Packed_8X8Y8Z2VertPos3Face), 3);
}

TEST(TestB, VertexPackingTests) {
    SpireVoxel::VertexData vertex = SpireVoxel::PackVertexData(0, 63, 64, 0, SpireVoxel::VoxelVertexPosition::THREE, 0);
    glm::uvec3 pos = SpireVoxel::UnpackVertexDataXYZ(vertex.Packed_8X8Y8Z2VertPos3Face);
    EXPECT_EQ(pos.x, 63);
    EXPECT_EQ(pos.y, 64);
    EXPECT_EQ(pos.z, 0);
    EXPECT_EQ(SpireVoxel::UnpackVertexDataVertexPosition(vertex.Packed_8X8Y8Z2VertPos3Face), SpireVoxel::VoxelVertexPosition::THREE);
    EXPECT_EQ(SpireVoxel::UnpackVertexDataFace(vertex.Packed_8X8Y8Z2VertPos3Face), 0);
}
