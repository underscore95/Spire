#include "EngineIncludes.h"
#include "../Assets/Shaders/ShaderInfo.h"
#include <gtest/gtest.h>

TEST(VertexPackingTests, BasicAssertions) {
    SpireVoxel::VertexData vertex = SpireVoxel::PackVertexData(0, 14, 100, 15, 0, 0, 0);
    glm::uvec3 pos = SpireVoxel::UnpackVertexDataXYZ(vertex.Packed_XYZ);
    EXPECT_EQ(pos.x, 14);
    EXPECT_EQ(pos.y, 100);
    EXPECT_EQ(pos.z, 15);
}
