#pragma once

#include "EngineIncludes.h"
#include "../../Assets/Shaders/ShaderInfo.h"

namespace SpireVoxel {
    static constexpr int VOXEL_TYPE_AIR = 0;

    class Chunk {
    public:
        explicit Chunk(Spire::RenderingManager &renderingManager, glm::ivec3 chunkPosition);

        ~Chunk();

        DISABLE_COPY(Chunk);

        Chunk(Chunk&& other) noexcept;

        Chunk& operator=(Chunk&& other) noexcept;

    public:
        void SetVoxel(glm::vec3 pos, std::int32_t type);

        void SetVoxelRect(glm::vec3 pos, glm::vec3 rectDimensions, std::int32_t type);

        void CmdBindIndexBuffer(
            VkCommandBuffer commandBuffer
        ) const;

        void CmdRender(VkCommandBuffer commandBuffer
        ) const;

        [[nodiscard]] std::optional<Spire::Descriptor> GetDescriptor(glm::u32 binding);

    private:
        void RegenerateMesh();

        void FreeBuffersIfAllocated();

    private:
        Spire::RenderingManager &m_renderingManager;

        glm::ivec3 m_chunkPosition;

        std::array<std::int32_t, SPIRE_VOXEL_CHUNK_VOLUME> m_voxelData{};

        Spire::VulkanBuffer m_vertexStorageBuffer;
        Spire::VulkanBuffer m_indexBuffer;
        size_t m_numIndices = 0;
    };
} // SpireVoxel
