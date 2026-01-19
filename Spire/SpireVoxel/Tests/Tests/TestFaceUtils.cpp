#include "EngineIncludes.h"
#include "../Assets/Shaders/ShaderInfo.h"
#include <gtest/gtest.h>
#include "TestHelpers.h"

TEST(FaceUtilsTests, TestDirectionToFaceA) {
    EXPECT_EQ(SpireVoxel::DirectionToFace(glm::vec3(0.3,0,0)), SPIRE_VOXEL_FACE_POS_X);
    EXPECT_EQ(SpireVoxel::DirectionToFace(glm::vec3(-1.3,0,0)), SPIRE_VOXEL_FACE_NEG_X);
    EXPECT_EQ(SpireVoxel::DirectionToFace(glm::vec3(0.3,1,0)), SPIRE_VOXEL_FACE_POS_Y);
    EXPECT_EQ(SpireVoxel::DirectionToFace(glm::vec3(0, -1,0)), SPIRE_VOXEL_FACE_NEG_Y);
    EXPECT_EQ(SpireVoxel::DirectionToFace(glm::vec3(0.3,0,0.31)), SPIRE_VOXEL_FACE_POS_Z);
    EXPECT_EQ(SpireVoxel::DirectionToFace(glm::vec3(0.3,40,-312)), SPIRE_VOXEL_FACE_NEG_Z);
    EXPECT_EQ(SpireVoxel::DirectionToFace(glm::vec3(0.3,-40,-312)), SPIRE_VOXEL_FACE_NEG_Z);

#ifndef NDEBUG
    EXPECT_DEATH(auto _ = SpireVoxel::DirectionToFace(glm::vec3(0,0,0));, "");
#endif
}
