#pragma once

#include "EngineIncludes.h"

namespace SpireVoxel {
    class VoxelWorld;

    // See documentation for format spec
    class VoxelSerializer {
    public:
        // Serialize the chunks into a directory, overwrites any existing chunks but if an existing chunk doesn't exist in the new world it is not overwritten.
        // directory does not need to exist.
        static void Serialize(VoxelWorld &world, const std::filesystem::path &directory);

        // Unload all chunks in a world. Load all chunks in a directory. Directory does not need to exist.
        static void ClearAndDeserialize(VoxelWorld &world, const std::filesystem::path &directory);

        static constexpr std::uint32_t VERSION = 1;

    private:
        static constexpr std::array<char, 10> HEADER_IDENTIFIER{'S', 'P', 'R', 'V', 'X', 'L', 'C', 'H', 'N', 'K'};
    };
} // SpireVoxel
