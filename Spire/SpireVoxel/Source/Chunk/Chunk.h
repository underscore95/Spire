#pragma once

#include "../Rendering/BufferAllocator.h"
#include "EngineIncludes.h"
#include "../../Assets/Shaders/ShaderInfo.h"

namespace SpireVoxel {
    class VoxelWorld;
}

namespace SpireVoxel {
    static constexpr int VOXEL_TYPE_AIR = 0;

    struct Chunk {
        glm::ivec3 ChunkPosition;
        VoxelWorld &World;
        std::array<std::int32_t, SPIRE_VOXEL_CHUNK_VOLUME> VoxelData{};
        std::uint64_t CorruptedMemoryCheck = 9238745897238972389; // This value will be changed if something overruns when editing VoxelData
        std::uint64_t CorruptedMemoryCheck2 = 12387732823748723; // This value will be changed if something overruns when editing MergedVoxels
        BufferAllocator::Allocation Allocation = {};
        glm::u32 NumVertices;

        std::vector<VertexData> GenerateMesh() const;

        [[nodiscard]] ChunkData GenerateChunkData(glm::u32 chunkIndex) const;

        [[nodiscard]] static std::optional<std::size_t> GetIndexOfVoxel(glm::ivec3 chunkPosition, glm::ivec3 voxelWorldPosition);

        bool IsCorrupted() const { return CorruptedMemoryCheck != 9238745897238972389 || CorruptedMemoryCheck2 != 12387732823748723; }

    };
} // SpireVoxel
