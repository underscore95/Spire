#pragma once

#include "EngineIncludes.h"

namespace SpireVoxel {
    // Specialised map from voxel position (uvec3) to voxel type (uint32)
    // Can be uploaded to GPU and queried
    class VoxelPositionToTypeHashMap {
    public:
        struct Entry {
            glm::u32 PosX;
            glm::u32 PosY;
            glm::u32 PosZ;
            glm::u32 Type;
        };

        // Construct a hash map from unordered_map
        explicit VoxelPositionToTypeHashMap(const std::unordered_map<glm::uvec3, glm::u32> &map);

    public:
        // Convert the hash map to bytes in a way that can be reconstructed on GPU
        // uint32 - number of buckets
        // uint32 - bucket size
        // all the buckets are laid out sequentially in memory, with each bucket being an array of Entry
        std::vector<glm::i8> ToBytes();

        // Lookup value return EMPTY_VALUE if key doesn't exist
        [[nodiscard]] glm::u32 Get(glm::uvec3 key) const;

        // 0-1 range, how memory efficient are we
        // 1 means the used storage is equal to the amount of memory if you just stored keys and values in an array
        // 0.5 means the used storage is double
        [[nodiscard]] float GetLoadFactor() const;

    public:
        // CPU implementation not really necessary but easier to test and debug
        [[nodiscard]] static glm::u32 GetFromBytes(glm::uvec3 key, const std::vector<glm::i8> &bytes);

    private:
        void Construct(const std::unordered_map<glm::uvec3, glm::u32> &map, glm::u32 bucketCount);

        bool Insert(Entry entry, glm::u32 bucketCount, glm::u32 hashFunction = 0);

    private:
        glm::u32 mNumEntries;
        std::vector<std::vector<Entry> > mBuckets;
    };
} // SpireVoxel
