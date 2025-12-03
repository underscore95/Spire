#include "VoxelPositionToTypeHashMap.h"

#include "../../Assets/Shaders/VoxelDataHashMap.h"

namespace SpireVoxel {
    VoxelPositionToTypeHashMap::VoxelPositionToTypeHashMap(const std::unordered_map<glm::uvec3, glm::u32> &map)
        : mNumEntries(map.size()) {
        Construct(map, 10);
    }

    std::vector<VoxelPositionToTypeHashMap::Entry> VoxelPositionToTypeHashMap::ToSparseVector() const {
        std::vector<Entry> entries;
        entries.reserve(mNumEntries);
        for (const auto &bucket : mBuckets) {
            for (const Entry &entry : bucket) {
                entries.push_back(entry);
            }
        }
        return entries;
    }

    glm::u32 VoxelPositionToTypeHashMap::Get(glm::uvec3 key) const {
        for (std::size_t hashFunction = 0; hashFunction < SPIRE_VOXEL_DATA_NUM_HASH_FUNCTIONS; hashFunction++) {
            glm::u32 bucketIndex = VoxelDataHashMapHash(key.x, key.y, key.z, hashFunction) % mBuckets.size();
            auto &bucket = mBuckets[bucketIndex];
            for (const Entry &entry : bucket) {
                if (entry.Type != SPIRE_VOXEL_DATA_EMPTY_VOXEL_TYPE && entry.PosX == key.x && entry.PosY == key.y && entry.PosZ == key.z) {
                    return entry.Type;
                }
            }
        }
        return SPIRE_VOXEL_DATA_EMPTY_VOXEL_TYPE;
    }

    float VoxelPositionToTypeHashMap::GetLoadFactor() const {
        std::size_t capacity = mBuckets.size() * mBuckets[0].size();
        return static_cast<float>(mNumEntries) / static_cast<float>(capacity); // approx 0.73664826 with 10k random entries
    }

    bool VoxelPositionToTypeHashMap::Insert(Entry entry, glm::u32 bucketCount, glm::u32 hashFunction) {
        if (hashFunction >= SPIRE_VOXEL_DATA_NUM_HASH_FUNCTIONS) return false; // invalid function

        glm::u32 bucketIndex = VoxelDataHashMapHash(entry.PosX, entry.PosY, entry.PosZ, hashFunction) % bucketCount;
        auto &bucket = mBuckets[bucketIndex];

        // insert into bucket
        for (auto &bucketEntry : bucket) {
            if (bucketEntry.Type == SPIRE_VOXEL_DATA_EMPTY_VOXEL_TYPE) {
                bucketEntry = entry;
                return true;
            }
        }

        // failed to insert, swap element
        Entry existingEntry = bucket.front();
        bucket[0] = entry;

        return Insert(existingEntry, bucketCount, hashFunction + 1);
    }

    void VoxelPositionToTypeHashMap::Construct(const std::unordered_map<glm::uvec3, glm::u32> &map, glm::u32 bucketCount) {
        // initialize
        mBuckets.resize(bucketCount);
        const glm::u32 bucketSize = glm::max(5u, static_cast<glm::u32>(glm::ceil(static_cast<float>(map.size()) / static_cast<float>(bucketCount))));

        for (auto &bucket : mBuckets) {
            bucket.resize(bucketSize);
            std::ranges::fill(bucket, Entry{
                                  .PosX = SPIRE_VOXEL_DATA_EMPTY_VOXEL_TYPE, .PosY = SPIRE_VOXEL_DATA_EMPTY_VOXEL_TYPE, .PosZ = SPIRE_VOXEL_DATA_EMPTY_VOXEL_TYPE,
                                  .Type = SPIRE_VOXEL_DATA_EMPTY_VOXEL_TYPE
                              });
        }

        // insert values
        for (const auto &[key, val] : map) {
            assert(val != SPIRE_VOXEL_DATA_EMPTY_VOXEL_TYPE);
            bool inserted = Insert({.PosX = key.x, .PosY = key.y, .PosZ = key.z, .Type = val}, bucketCount);

            // failed to find a free spot in the bucket
            if (!inserted) {
                assert(bucketCount < map.size());
                auto newBucketCount = static_cast<glm::u32>(static_cast<float>(bucketCount) * 1.25);
                Spire::warn("Failed to construct map with {} elements and {} buckets, retrying with {} buckets", map.size(), bucketCount, newBucketCount);
                Construct(map, newBucketCount);
                return;
            }
        }
    }
} // SpireVoxel
