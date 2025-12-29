#pragma once

#include "../Rendering/BufferAllocator.h"
#include "EngineIncludes.h"
#include "../../Assets/Shaders/ShaderInfo.h"

namespace SpireVoxel {
    class VoxelWorld;
}

namespace SpireVoxel {
    static constexpr int VOXEL_TYPE_AIR = 0;

    struct ChunkMesh {
        std::vector<VertexData> Vertices;
    };

    struct Chunk {
        glm::ivec3 ChunkPosition;
        VoxelWorld &World;
        std::array<std::uint32_t, SPIRE_VOXEL_CHUNK_VOLUME> VoxelData{};
        std::uint64_t CorruptedMemoryCheck = 9238745897238972389; // This value will be changed if something overruns when editing VoxelData
        BufferAllocator::Allocation VertexAllocation = {};
        glm::u32 NumVertices;

        void SetVoxel(glm::u32 index, glm::u32 type);

        void SetVoxels(glm::u32 startIndex, glm::u32 endIndex, glm::u32 type);

        [[nodiscard]] ChunkMesh GenerateMesh() const;

        [[nodiscard]] ChunkData GenerateChunkData(glm::u32 chunkIndex) const;

        [[nodiscard]] static std::optional<std::size_t> GetIndexOfVoxel(glm::ivec3 chunkPosition, glm::ivec3 voxelWorldPosition);

        [[nodiscard]] bool IsCorrupted() const { return CorruptedMemoryCheck != 9238745897238972389; }

    private:
        void GreedyMesh(ChunkMesh &mesh, glm::u32 voxelType) const;

        void PushFace(std::vector<VertexData> &vertices, glm::u32 face, glm::uvec3 p, glm::u32 width, glm::u32 height, glm::u32 voxelType) const;
    };
} // SpireVoxel
