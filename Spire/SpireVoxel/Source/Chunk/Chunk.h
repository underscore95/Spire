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
        std::array<std::int32_t, SPIRE_VOXEL_CHUNK_VOLUME> VoxelData{};
        BufferAllocator::Allocation Allocation;
        glm::u32 NumVertices;

        std::vector<VertexData> GenerateMesh();

        [[nodiscard]] ChunkData GenerateChunkData(glm::u32 chunkIndex) const;
    };
} // SpireVoxel
