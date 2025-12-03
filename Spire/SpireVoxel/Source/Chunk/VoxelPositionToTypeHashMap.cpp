#include "VoxelPositionToTypeHashMap.h"

namespace SpireVoxel {
    VoxelPositionToTypeHashMap::VoxelPositionToTypeHashMap(const std::unordered_map<glm::uvec3, glm::u32> &map)
        : mNumEntries(map.size()) {
        Construct(map, 10);
        Spire::info("{}", GetLoadFactor());
    }

    std::vector<glm::i8> VoxelPositionToTypeHashMap::ToBytes() {
        glm::u32 numBuckets = mBuckets.size();
        glm::u32 bucketSize = mBuckets[0].size();

        std::vector<glm::i8> bytes(sizeof(numBuckets) + sizeof(bucketSize) + (numBuckets * bucketSize * sizeof(Entry)));
        std::size_t writeIndex = 0;
        memcpy(bytes.data() + writeIndex, &numBuckets, sizeof(numBuckets));
        writeIndex += sizeof(numBuckets);
        memcpy(bytes.data() + writeIndex, &bucketSize, sizeof(bucketSize));
        writeIndex += sizeof(bucketSize);

        for (auto &bucket : mBuckets) {
            std::size_t dataSize = bucket.size() * sizeof(Entry);
            memcpy(bytes.data() + writeIndex, bucket.data(), dataSize);
            writeIndex += dataSize;
        }

        return bytes;
    }

    glm::u32 VoxelPositionToTypeHashMap::Get(glm::uvec3 key) const {
        for (std::size_t hashFunction = 0; hashFunction < HASH_COEFFICIENTS.size(); hashFunction++) {
            glm::u32 bucketIndex = Hash({.PosX = key.x, .PosY = key.y, .PosZ = key.z}, hashFunction) % mBuckets.size();
            auto &bucket = mBuckets[bucketIndex];
            for (const Entry &entry : bucket) {
                if (entry.Type != EMPTY_VALUE && entry.PosX == key.x && entry.PosY == key.y && entry.PosZ == key.z) {
                    return entry.Type;
                }
            }
        }
        return EMPTY_VALUE;
    }

    float VoxelPositionToTypeHashMap::GetLoadFactor() const {
        std::size_t capacity = mBuckets.size() * mBuckets[0].size();
        return static_cast<float>(mNumEntries) / static_cast<float>(capacity); // approx 0.73664826 with 10k random entries
    }

    glm::u32 VoxelPositionToTypeHashMap::GetFromBytes(glm::uvec3 key, const std::vector<glm::i8> &bytes) {
        std::size_t readIndex = 0;
        glm::u32 numBuckets = *reinterpret_cast<const glm::u32 *>(bytes.data() + readIndex);
        readIndex += sizeof(numBuckets);
        glm::u32 bucketSize = *reinterpret_cast<const glm::u32 *>(bytes.data() + readIndex);
        readIndex += sizeof(bucketSize);


        for (std::size_t hashFunction = 0; hashFunction < HASH_COEFFICIENTS.size(); hashFunction++) {
            glm::u32 bucketIndex = Hash({.PosX = key.x, .PosY = key.y, .PosZ = key.z}, hashFunction) % numBuckets;
            std::size_t bucketLocation = readIndex + sizeof(Entry) * bucketSize * bucketIndex;
            auto bucket = reinterpret_cast<const Entry *>(bytes.data() + bucketLocation);
            for (std::size_t i = 0; i < bucketSize; i++) {
                const Entry &entry = bucket[i];
                if (entry.Type != EMPTY_VALUE && entry.PosX == key.x && entry.PosY == key.y && entry.PosZ == key.z) {
                    return entry.Type;
                }
            }
        }

        return EMPTY_VALUE;
    }

    bool VoxelPositionToTypeHashMap::Insert(Entry entry, glm::u32 bucketCount, glm::u32 hashFunction) {
        if (hashFunction >= HASH_COEFFICIENTS.size()) return false; // invalid function

        glm::u32 bucketIndex = Hash(entry, hashFunction) % bucketCount;
        auto &bucket = mBuckets[bucketIndex];

        // insert into bucket
        for (std::size_t i = 0; i < bucket.size(); i++) {
            if (bucket[i].Type == EMPTY_VALUE) {
                bucket[i] = entry;
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
            std::ranges::fill(bucket, Entry{.PosX = EMPTY_VALUE, .PosY = EMPTY_VALUE, .PosZ = EMPTY_VALUE, .Type = EMPTY_VALUE});
        }

        // insert values
        for (const auto &[key, val] : map) {
            assert(val != EMPTY_VALUE);
            bool inserted = Insert({.PosX = key.x, .PosY = key.y, .PosZ = key.z, .Type = val}, bucketCount);

            // failed to find a free spot in the bucket
            if (!inserted) {
                assert(bucketCount < map.size());
                glm::u32 newBucketCount = bucketCount * 1.25;
                Spire::warn("Failed to construct map with {} elements and {} buckets, retrying with {} buckets", map.size(), bucketCount, newBucketCount);
                Construct(map, newBucketCount);
                return;
            }
        }
    }

    glm::u32 VoxelPositionToTypeHashMap::Hash(Entry entry, glm::u32 hashFunction) {
        return (entry.PosX * HASH_COEFFICIENTS[hashFunction].x) ^ (entry.PosY * HASH_COEFFICIENTS[hashFunction].y) ^ (entry.PosZ * HASH_COEFFICIENTS[hashFunction].z);
    }
} // SpireVoxel
