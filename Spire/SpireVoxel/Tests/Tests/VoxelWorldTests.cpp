#include "EngineIncludes.h"
#include "../Assets/Shaders/ShaderInfo.h"
#include <gtest/gtest.h>

#include "Chunk/VoxelWorld.h"

TEST(VertexWorldTests, TestGetChunkPositionOfVoxel) {
    EXPECT_EQ(glm::ivec3(0,0,0), SpireVoxel::VoxelWorld::GetChunkPositionOfVoxel(glm::ivec3{0,0,0}));
    EXPECT_EQ(glm::ivec3(0,0,0), SpireVoxel::VoxelWorld::GetChunkPositionOfVoxel(glm::ivec3{4,0,0}));
    EXPECT_EQ(glm::ivec3(-1,0,0), SpireVoxel::VoxelWorld::GetChunkPositionOfVoxel(glm::ivec3{-1,0,0}));
    EXPECT_EQ(glm::ivec3(0,1,1), SpireVoxel::VoxelWorld::GetChunkPositionOfVoxel(glm::ivec3{0, SPIRE_VOXEL_CHUNK_SIZE, SPIRE_VOXEL_CHUNK_SIZE + 13}));
}

TEST(VertexWorldTests, TestGetWorldVoxelPositionInChunk) {
    EXPECT_EQ(glm::ivec3(0,0,0), SpireVoxel::VoxelWorld::GetWorldVoxelPositionInChunk({0,0,0}, {0, 0, 0}));
    EXPECT_EQ(glm::ivec3(0,5,0), SpireVoxel::VoxelWorld::GetWorldVoxelPositionInChunk({0,0,0}, {0, 5, 0}));
    EXPECT_EQ(glm::ivec3(0,5 + SPIRE_VOXEL_CHUNK_SIZE,0), SpireVoxel::VoxelWorld::GetWorldVoxelPositionInChunk({0,1,0}, {0, 5 , 0}));
    EXPECT_EQ(glm::ivec3(3 * SPIRE_VOXEL_CHUNK_SIZE + 14, 5 * SPIRE_VOXEL_CHUNK_SIZE + 3, 9 * SPIRE_VOXEL_CHUNK_SIZE + 12),
              SpireVoxel::VoxelWorld::GetWorldVoxelPositionInChunk({3, 5, 9}, {14,3, 12}));
}
