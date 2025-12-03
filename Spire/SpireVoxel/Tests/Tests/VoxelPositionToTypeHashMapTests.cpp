#include "EngineIncludes.h"
#include <gtest/gtest.h>
#include "TestHelpers.h"
#include "Chunk/VoxelPositionToTypeHashMap.h"

TEST(VoxelPositionToTypeHashMap, TestA) {
    // Construct random unordered_map
    std::unordered_map<glm::uvec3, glm::u32> m;
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<uint32_t> dist;

    m.reserve(100);
    for (int i = 0; i < 100; i++) {
        glm::uvec3 key(dist(rng), dist(rng), dist(rng));
        glm::u32 value = dist(rng);
        if (value == SpireVoxel::VoxelPositionToTypeHashMap::EMPTY_VALUE) value++;
        m.emplace(key, value);
    }

    // Construct hash map
    SpireVoxel::VoxelPositionToTypeHashMap hashMap(m);
    std::vector<glm::i8> bytes = hashMap.ToBytes();

    // Test present
    for (const auto &[key, val] : m) {
        EXPECT_EQ(hashMap.Get(key), val);
        EXPECT_EQ(hashMap.Get(key), SpireVoxel::VoxelPositionToTypeHashMap::GetFromBytes(key, bytes));
    }

    // Test non-present
    for (int i = 0; i < 100; i++) {
        glm::uvec3 key(dist(rng), dist(rng), dist(rng));
        EXPECT_EQ(hashMap.Get(key) != SpireVoxel::VoxelPositionToTypeHashMap::EMPTY_VALUE, m.contains(key));
        EXPECT_EQ(hashMap.Get(key), SpireVoxel::VoxelPositionToTypeHashMap::GetFromBytes(key, bytes));
    }
}
