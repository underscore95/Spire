#include "EngineIncludes.h"
#include "../Assets/Shaders/ShaderInfo.h"
#include <gtest/gtest.h>

TEST(VertexPackingTests, TestA) {
    SpireVoxel::VertexData vertex = SpireVoxel::PackVertexData(5, 1, 14, 50, 15, SpireVoxel::VoxelVertexPosition::ZERO, 3);
    glm::uvec3 pos = SpireVoxel::UnpackVertexDataXYZ(vertex.Packed_7X7Y7Z2VertPos3Face);
    EXPECT_EQ(pos.x, 14);
    EXPECT_EQ(pos.y, 50);
    EXPECT_EQ(pos.z, 15);
    EXPECT_EQ(SpireVoxel::UnpackVertexDataVertexPosition(vertex.Packed_7X7Y7Z2VertPos3Face), SpireVoxel::VoxelVertexPosition::ZERO);
    EXPECT_EQ(SpireVoxel::UnpackVertexDataFace(vertex.Packed_7X7Y7Z2VertPos3Face), 3);
    EXPECT_EQ(SpireVoxel::UnpackVertexFaceWidthHeight(vertex.Packed_6Width6Height).x, 5);
    EXPECT_EQ(SpireVoxel::UnpackVertexFaceWidthHeight(vertex.Packed_6Width6Height).y, 1);
}

TEST(VertexPackingTests, TestB) {
    SpireVoxel::VertexData vertex = SpireVoxel::PackVertexData(63, 19, 63, 64, 0, SpireVoxel::VoxelVertexPosition::THREE, 0);
    glm::uvec3 pos = SpireVoxel::UnpackVertexDataXYZ(vertex.Packed_7X7Y7Z2VertPos3Face);
    EXPECT_EQ(pos.x, 63);
    EXPECT_EQ(pos.y, 64);
    EXPECT_EQ(pos.z, 0);
    EXPECT_EQ(SpireVoxel::UnpackVertexDataVertexPosition(vertex.Packed_7X7Y7Z2VertPos3Face), SpireVoxel::VoxelVertexPosition::THREE);
    EXPECT_EQ(SpireVoxel::UnpackVertexDataFace(vertex.Packed_7X7Y7Z2VertPos3Face), 0);
    EXPECT_EQ(SpireVoxel::UnpackVertexFaceWidthHeight(vertex.Packed_6Width6Height).x, 63);
    EXPECT_EQ(SpireVoxel::UnpackVertexFaceWidthHeight(vertex.Packed_6Width6Height).y, 19);
}
