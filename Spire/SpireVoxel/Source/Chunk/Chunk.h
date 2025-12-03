#pragma once

#include <bitset>

#include "../Rendering/BufferAllocator.h"
#include "EngineIncludes.h"
#include "VoxelPositionToTypeHashMap.h"
#include "../../Assets/Shaders/ShaderInfo.h"

namespace SpireVoxel {
    class VoxelWorld;
}

namespace SpireVoxel {
    static constexpr int VOXEL_TYPE_AIR = 0;

    struct ChunkMesh {
        std::vector<VertexData> Vertices;
        std::unique_ptr<VoxelPositionToTypeHashMap> VoxelDataHashMap;
    };

    struct Chunk {
        glm::ivec3 ChunkPosition;
        VoxelWorld &World;
        std::array<std::uint32_t, SPIRE_VOXEL_CHUNK_VOLUME> VoxelData{};
        std::uint64_t CorruptedMemoryCheck = 9238745897238972389; // This value will be changed if something overruns when editing VoxelData
        std::bitset<SPIRE_VOXEL_CHUNK_VOLUME> VoxelBits{}; // 1 = voxel is present, 0 = voxel is empty
        std::uint64_t CorruptedMemoryCheck2 = 12387732823748723; // This value will be changed if something overruns when editing VoxelBits
        BufferAllocator::Allocation VertexAllocation = {};
        BufferAllocator::Allocation VoxelDataAllocation = {};
        glm::u32 NumVertices;
        glm::u32 VoxelDataMapBucketCount = 0;

        void SetVoxel(glm::u32 index, glm::u32 type);

        void SetVoxels(glm::u32 startIndex, glm::u32 endIndex, glm::u32 type);

        [[nodiscard]] ChunkMesh GenerateMesh() const;

        [[nodiscard]] ChunkData GenerateChunkData(glm::u32 chunkIndex) const;

        void RegenerateVoxelBits();

        [[nodiscard]] static std::optional<std::size_t> GetIndexOfVoxel(glm::ivec3 chunkPosition, glm::ivec3 voxelWorldPosition);

        [[nodiscard]] bool IsCorrupted() const { return CorruptedMemoryCheck != 9238745897238972389 || CorruptedMemoryCheck2 != 12387732823748723; }

    private:
        void InsertVoxelData(std::unordered_map<glm::uvec3, glm::u32> &voxelData, glm::uvec3 start, glm::uvec3 dimensions) const;

        void PushFace(std::vector<VertexData> &vertices, std::unordered_map<glm::uvec3, glm::u32> &voxelData, glm::u32 face, glm::uvec3 p, glm::u32 width, glm::u32 height) const;
    };
} // SpireVoxel
