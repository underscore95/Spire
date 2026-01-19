#pragma once

#include "EngineIncludes.h"

namespace SpireVoxel {
    class VoxelWorld;

    // See documentation (root directory of repo) for format spec
    class VoxelSerializer {
    public:
        // Serialize the chunks into a directory, overwrites any existing chunks but if an existing chunk doesn't exist in the new world it is not overwritten.
        // directory does not need to exist.
        static void Serialize(VoxelWorld &world, const std::filesystem::path &directory);

        // Unload all chunks in a world. Load all chunks in a directory. Directory does not need to exist.
        static void ClearAndDeserialize(VoxelWorld &world, const std::filesystem::path &directory);

        static constexpr std::uint32_t VERSION = 2;
        static constexpr std::uint32_t VOXEL_TYPE_U32_TO_U16_VERSION = 2; // this version changes voxel types from u32 to u16

    private:
        static constexpr std::array<char, 10> HEADER_IDENTIFIER{'S', 'P', 'R', 'V', 'X', 'L', 'C', 'H', 'N', 'K'};
    };
} // SpireVoxel
