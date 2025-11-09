#pragma once

#include "EngineIncludes.h"
#include "../../Assets/Shaders/ShaderInfo.h"

namespace SpireVoxel {
    class VoxelWorld;
}

namespace SpireVoxel {
    static constexpr int VOXEL_TYPE_AIR = 0;

    class Chunk {
    public:
        explicit Chunk(Spire::RenderingManager &renderingManager, glm::ivec3 chunkPosition, VoxelWorld& world);

        ~Chunk();

        DISABLE_COPY(Chunk);

        Chunk(Chunk &&other) noexcept;

        Chunk &operator=(Chunk &&other) noexcept;

    public:
        void SetVoxel(glm::vec3 pos, std::int32_t type);

        void SetVoxelRect(glm::vec3 pos, glm::vec3 rectDimensions, std::int32_t type);

        [[nodiscard]] const Spire::VulkanBuffer& GetVertexBuffer() const;

        [[nodiscard]] bool HasMesh() const;

        [[nodiscard]] ChunkData GetChunkData() const;

    private:
        void OnChunkEdited();
        void RegenerateMesh();

        void FreeBuffersIfAllocated();

    private:
        Spire::RenderingManager &m_renderingManager;

        glm::ivec3 m_chunkPosition;

        std::array<std::int32_t, SPIRE_VOXEL_CHUNK_VOLUME> m_voxelData{};

        Spire::VulkanBuffer m_vertexStorageBuffer; // todo keep old buffers around until in flight frames are done with them, but in a way where we don't need 3 meshes all the time

        VoxelWorld& m_world;
    };
} // SpireVoxel
