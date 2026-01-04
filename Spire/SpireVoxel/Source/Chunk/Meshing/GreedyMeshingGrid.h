#pragma once

#include "EngineIncludes.h"
#include "../../../Assets/Shaders/ShaderInfo.h"

namespace SpireVoxel {
    class GreedyMeshingGrid {
    public:
        // Will read SPIRE_VOXEL_CHUNK_SIZE elements from array
        explicit GreedyMeshingGrid(glm::u64* bits);

    public:
        void SetBit(glm::u32 row, glm::u32 col);

        bool GetBit(glm::u32 row, glm::u32 col) const;

        void SetEmptyVoxels(glm::u32 col, glm::u32 row, glm::u32 height);

        [[nodiscard]] glm::u64 GetColumn(glm::u32 column) const { return m_bits[column]; }

        // maps slice, a, b coordinates into chunk coords
        [[nodiscard]] static glm::uvec3 GetChunkCoords(glm::u32 slice, glm::u32 row, glm::u32 col, glm::u32 face);

        // Count the number of trailing 0s (empty) or 1s (present)
        // Column is right shifted startingRow bits beforehand, this can be used to ignore some bits
        // startingRow should be the first bit you want counted, e.g. if you have 000110 and want to count trailing 1s, startingRow should be 1
        [[nodiscard]] glm::u64 NumTrailingEmptyVoxels(glm::u32 column, glm::u32 startingRow) const {
            return std::countr_zero(m_bits[column] >> startingRow);
        }

        [[nodiscard]] glm::u64 NumTrailingPresentVoxels(glm::u32 column, glm::u32 startingRow) const {
            return std::countr_one(m_bits[column] >> startingRow);
        }

        [[nodiscard]] const glm::u64* GetBitmask() const { return m_bits; }

        void Print() const;

    private:
        static_assert(SPIRE_VOXEL_CHUNK_SIZE == 64); // since u64 used
        glm::u64* m_bits;
    };
} // SpireVoxel