#pragma once

#include "EngineIncludes.h"

namespace SpireVoxel {
    class VoxelWorldRenderer;
}

namespace SpireVoxel {
    struct Chunk;

    class VoxelWorld {
    public:
        explicit VoxelWorld(Spire::RenderingManager &renderingManager);

    public:
        Chunk &LoadChunk(glm::ivec3 chunkPosition);

        void LoadChunks(const std::vector<glm::ivec3> &chunkPositions);

        void UnloadChunks(const std::vector<glm::ivec3> &chunkPositions);

        [[nodiscard]] std::shared_ptr<Chunk> GetLoadedChunk(glm::ivec3 chunkPosition);

        [[nodiscard]] const std::shared_ptr<Chunk> GetLoadedChunk(glm::ivec3 chunkPosition) const;

        [[nodiscard]] bool IsLoaded(const Chunk &chunk);

        [[nodiscard]] std::size_t NumLoadedChunks() const;

        std::unordered_map<glm::ivec3, std::shared_ptr<Chunk>>::iterator begin();

        std::unordered_map<glm::ivec3, std::shared_ptr<Chunk>>::iterator end();

        [[nodiscard]] std::uint32_t GetNumLoadedChunks() const;

        void UnloadAllChunks();

        [[nodiscard]] VoxelWorldRenderer &GetRenderer() const;

        [[nodiscard]] glm::u64 CalculateGPUMemoryUsageForChunks() const;

        [[nodiscard]] glm::u64 CalculateCPUMemoryUsageForChunks() const;

        // returns true if a voxel is present at the world position
        // returns false if the voxel is air or the chunk is not loaded
        [[nodiscard]] bool IsVoxelAt(glm::ivec3 worldPosition) const;

        glm::u32 GetVoxelAt(glm::ivec3 worldPosition) const;

        bool TrySetVoxelAt(glm::ivec3 worldPosition, glm::u32 voxelType);

        [[nodiscard]] static glm::ivec3 GetChunkPositionOfVoxel(glm::ivec3 voxelWorldPosition);

        [[nodiscard]] static glm::ivec3 GetWorldVoxelPositionInChunk(glm::ivec3 chunkPosition, glm::uvec3 voxelPositionInChunk);

        [[nodiscard]] static glm::uvec3 ToChunkSpace(glm::ivec3 worldVoxelPosition);

    private:
        // https://en.cppreference.com/w/cpp/container/unordered_map.html - always iterates in the same order if the map hasnt been changed
        std::unordered_map<glm::ivec3, std::shared_ptr<Chunk>> m_chunks;
        std::unique_ptr<VoxelWorldRenderer> m_renderer;
    };
} // SpireVoxel
