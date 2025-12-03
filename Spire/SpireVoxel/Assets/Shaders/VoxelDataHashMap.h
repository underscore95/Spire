// ReSharper disable CppVariableCanBeMadeConstexpr
#ifndef SPIRE_VOXEL_DATA_HASH_MAP
#define SPIRE_VOXEL_DATA_HASH_MAP

#include "ShaderInfo.h"

#ifdef __cplusplus
namespace SpireVoxel {
#endif

#define SPIRE_VOXEL_DATA_NUM_HASH_FUNCTIONS 10
#define SPIRE_VOXEL_DATA_EMPTY_VOXEL_TYPE 0

    // hash coefficients
    SPIRE_KEYWORD_STATIC const SPIRE_UVEC3_TYPE VOXEL_DATA_HASH_COEFFICIENTS[SPIRE_VOXEL_DATA_NUM_HASH_FUNCTIONS] =
#ifdef __cplusplus
    {
#else
    SPIRE_UVEC3_TYPE[](
#endif
        SPIRE_UVEC3_TYPE(1061060661, 855378910, 3502893387),
        SPIRE_UVEC3_TYPE(2383498209, 1644277160, 2749756682),
        SPIRE_UVEC3_TYPE(671433660, 814371953, 3950429538),
        SPIRE_UVEC3_TYPE(1934605449, 3143018405, 2436686234),
        SPIRE_UVEC3_TYPE(41216935, 2734168707, 2265436213),
        SPIRE_UVEC3_TYPE(2965107964, 3434689208, 2616134055),
        SPIRE_UVEC3_TYPE(816425089, 1132258741, 1382123267),
        SPIRE_UVEC3_TYPE(373591499, 1250552626, 497653521),
        SPIRE_UVEC3_TYPE(1892588271, 3869359181, 4038477593),
        SPIRE_UVEC3_TYPE(3867437441, 2157261722, 3688971228)

#ifdef __cplusplus
    };
#else
    );
#endif

    // HashMap buffer
#ifndef  __cplusplus
    struct VoxelDataHashMapEntry {
        SPIRE_UINT32_TYPE PosX;
        SPIRE_UINT32_TYPE PosY;
        SPIRE_UINT32_TYPE PosZ;
        SPIRE_UINT32_TYPE VoxelType;
    };

    layout (set = SPIRE_VOXEL_SHADER_BINDINGS_CONSTANT_CHUNK_SET, binding = SPIRE_VOXEL_SHADER_BINDINGS_VOXEL_DATA_MAP_ENTRIES) readonly buffer VoxelDataHashMapEntriesBuffer {
        VoxelDataHashMapEntry entries[];
    } voxelDataHashMapEntries;
#endif

    // hash function for voxel data
    // hashes key x y z into uint32
    SPIRE_KEYWORD_INLINE SPIRE_KEYWORD_NODISCARD SPIRE_UINT32_TYPE VoxelDataHashMapHash(
        SPIRE_UINT32_TYPE x, SPIRE_UINT32_TYPE y, SPIRE_UINT32_TYPE z,
        SPIRE_UINT32_TYPE hashFunction
    ) {
        return (x * VOXEL_DATA_HASH_COEFFICIENTS[hashFunction].x) ^
               (y * VOXEL_DATA_HASH_COEFFICIENTS[hashFunction].y) ^
               (z * VOXEL_DATA_HASH_COEFFICIENTS[hashFunction].z);
    }

    // GLSL HashMap Query
#ifndef __cplusplus
    uint VoxelDataHashMapGet(uint x, uint y, uint z, uint startingMapIndex, uint bucketSize, uint bucketCount) {
        for (uint hashFunction = 0; hashFunction < SPIRE_VOXEL_DATA_NUM_HASH_FUNCTIONS; hashFunction++) {
            uint bucketIndex = VoxelDataHashMapHash(x, y, z, hashFunction) % bucketCount;
            uint startingEntryIndex = bucketSize * bucketIndex + startingMapIndex; // first entry in our bucket
            for (uint entryIndex = startingEntryIndex; entryIndex < startingEntryIndex + bucketSize; entryIndex++) {

                if (voxelDataHashMapEntries.entries[entryIndex].VoxelType != SPIRE_VOXEL_DATA_EMPTY_VOXEL_TYPE
                    && voxelDataHashMapEntries.entries[entryIndex].PosX == x
                    && voxelDataHashMapEntries.entries[entryIndex].PosY == y
                    && voxelDataHashMapEntries.entries[entryIndex].PosZ == z
                    ) {
                    return voxelDataHashMapEntries.entries[entryIndex].VoxelType;
                }
            }
        }

        return SPIRE_VOXEL_DATA_EMPTY_VOXEL_TYPE;
    }
#endif

#ifdef __cplusplus
}
#endif
#endif
