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

    public:
        static constexpr glm::u32 EMPTY_VALUE = 0;
        static constexpr std::array<glm::uvec3, 10> HASH_COEFFICIENTS = {
            glm::uvec3{1061060661, 855378910, 3502893387},
            glm::uvec3{2383498209, 1644277160, 2749756682},
            glm::uvec3{671433660, 814371953, 3950429538},
            glm::uvec3{1934605449, 3143018405, 2436686234},
            glm::uvec3{41216935, 2734168707, 2265436213},
            glm::uvec3{2965107964, 3434689208, 2616134055},
            glm::uvec3{816425089, 1132258741, 1382123267},
            glm::uvec3{373591499, 1250552626, 497653521},
            glm::uvec3{1892588271, 3869359181, 4038477593},
            glm::uvec3{3867437441, 2157261722, 3688971228},
        };

    private:
        void Construct(const std::unordered_map<glm::uvec3, glm::u32> &map, glm::u32 bucketCount);

        bool Insert(Entry entry, glm::u32 bucketCount, glm::u32 hashFunction = 0);

        [[nodiscard]] static glm::u32 Hash(Entry entry, glm::u32 hashFunction);

    private:
        glm::u32 mNumEntries;
        std::vector<std::vector<Entry> > mBuckets;
    };
} // SpireVoxel
