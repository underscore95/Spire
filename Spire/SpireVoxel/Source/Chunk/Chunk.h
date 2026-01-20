#pragma once

#include <bitset>

#include "../Rendering/BufferAllocator.h"
#include "EngineIncludes.h"
#include "VoxelType.h"
#include "../../Assets/Shaders/ShaderInfo.h"

namespace SpireVoxel {
    struct ChunkMesh;
}

namespace SpireVoxel {
    class VoxelWorld;
}

namespace SpireVoxel {
    static constexpr int VOXEL_TYPE_AIR = 0;

    // Represents a 64^3 chunk of a world
    struct Chunk {
        glm::ivec3 ChunkPosition;
        VoxelWorld &World;
        std::array<VoxelType, SPIRE_VOXEL_CHUNK_VOLUME> VoxelData{};
        static_assert(sizeof(VoxelData) == sizeof(GPUChunkVoxelData));
        std::uint64_t CorruptedMemoryCheck = 9238745897238972389; // This value will be changed if something overruns when editing VoxelData
        std::bitset<SPIRE_VOXEL_CHUNK_VOLUME> VoxelBits{}; // 1 = voxel is present, 0 = voxel is empty
        std::uint64_t CorruptedMemoryCheck2 = 12387732823748723; // This value will be changed if something overruns when editing VoxelBits
        BufferAllocator::Allocation VertexAllocation = {};
        BufferAllocator::Allocation VoxelDataAllocation = {};
        glm::u32 NumVertices;

        void SetVoxel(glm::u32 index, VoxelType type);

        void SetVoxels(glm::u32 startIndex, glm::u32 endIndex, VoxelType type);

        [[nodiscard]] ChunkMesh GenerateMesh() const;

        [[nodiscard]] ChunkData GenerateChunkData(glm::u32 chunkIndex, glm::u32 numVerticesPerVertexBuffer) const;

        void RegenerateVoxelBits();

        [[nodiscard]] static std::optional<std::size_t> GetIndexOfVoxel(glm::ivec3 chunkPosition, glm::ivec3 voxelWorldPosition);

        [[nodiscard]] bool IsCorrupted() const { return CorruptedMemoryCheck != 9238745897238972389 || CorruptedMemoryCheck2 != 12387732823748723; }
    };
} // SpireVoxel
