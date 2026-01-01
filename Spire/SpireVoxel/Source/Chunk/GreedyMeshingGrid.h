#pragma once

#include "EngineIncludes.h"
#include "../../Assets/Shaders/ShaderInfo.h"

namespace SpireVoxel {
    class GreedyMeshingGrid {
    public:
        GreedyMeshingGrid() = default;

        // Copy SPIRE_VOXEL_CHUNK_SIZE elements from source starting at index into the grid to start
        GreedyMeshingGrid(const std::vector<glm::u64> &source, std::size_t index);

    public:
        void SetBit(glm::u32 row, glm::u32 col);

        bool GetBit(glm::u32 row, glm::u32 col) const;

        void SetEmptyVoxels(glm::u32 col, glm::u32 row, glm::u32 height);

        glm::u64 GetColumn(glm::u32 column) const { return m_bits[column]; }

        // maps slice, a, b coordinates into chunk coords
        [[nodiscard]] static glm::uvec3 GetChunkCoords(glm::u32 slice, glm::u32 row, glm::u32 col, glm::u32 face);

        // Count the number of trailing 0s (empty) or 1s (present)
        // Column is right shifted startingRow bits beforehand, this can be used to ignore some bits
        // startingRow should be the first bit you want counted, e.g. if you have 000110 and want to count trailing 1s, startingRow should be 1
        glm::u64 NumTrailingEmptyVoxels(glm::u32 column, glm::u32 startingRow) const {
            return std::countr_zero(m_bits[column] >> startingRow);
        }

        glm::u64 NumTrailingPresentVoxels(glm::u32 column, glm::u32 startingRow) const {
            return std::countr_one(m_bits[column] >> startingRow);
        }

        const std::array<glm::u64, SPIRE_VOXEL_CHUNK_SIZE> &GetBitmask() const { return m_bits; }

        void Print() const;

    private:
        static_assert(SPIRE_VOXEL_CHUNK_SIZE == 64); // since u64 used
        std::array<glm::u64, SPIRE_VOXEL_CHUNK_SIZE> m_bits = {}; // each u64 represents a column of voxels
    };
} // SpireVoxel
