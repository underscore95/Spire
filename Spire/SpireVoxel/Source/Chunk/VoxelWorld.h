#pragma once

#include "EngineIncludes.h"
#include "VoxelType.h"
#include "Generation/ProceduralGenerationManager.h"
#include "LOD/ISamplingOffsets.h"
#include "LOD/LODManager.h"

namespace SpireVoxel {
    class VoxelWorldRenderer;
}

namespace SpireVoxel {
    struct Chunk;

    class VoxelWorld {
        friend class VoxelRenderer;

    public:
        explicit VoxelWorld(
            Spire::Engine &engine,
            const std::shared_ptr<ISamplingOffsets> &samplingOffsets,
            const std::function<void()> &recreatePipelineCallback,
            bool isProfilingMeshing,
            std::unique_ptr<IProceduralGenerationProvider> provider,
            std::unique_ptr<IProceduralGenerationController> controller,
            IVoxelCamera &camera
        );

    public:
        // Handles level of detail for the world
        [[nodiscard]] LODManager &GetLODManager() const;

        // Handles what chunks to generate/load
        [[nodiscard]] ProceduralGenerationManager &GetProceduralGenerationManager() const;

        [[nodiscard]] Chunk &LoadChunk(glm::ivec3 chunkPosition);

        void LoadChunks(const std::vector<glm::ivec3> &chunkPositions);

        void UnloadChunks(const std::vector<glm::ivec3> &chunkPositions);

        // Get chunk if loaded
        // Use LODManager if using LODs
        [[nodiscard]] Chunk *TryGetLoadedChunk(glm::ivec3 chunkPosition);

        [[nodiscard]] const Chunk *TryGetLoadedChunk(glm::ivec3 chunkPosition) const;

        [[nodiscard]] bool IsLoaded(const Chunk &chunk);

        [[nodiscard]] std::size_t NumLoadedChunks() const;

        // Iterate over all loaded chunks
        std::unordered_map<glm::ivec3, std::unique_ptr<Chunk> >::iterator begin();

        std::unordered_map<glm::ivec3, std::unique_ptr<Chunk> >::iterator end();

        void UnloadAllChunks();

        [[nodiscard]] VoxelWorldRenderer &GetRenderer() const;

        // Approx calculate memory usage, only considers the big stuff
        [[nodiscard]] glm::u64 CalculateGPUMemoryUsageForChunks() const;

        [[nodiscard]] glm::u64 CalculateCPUMemoryUsageForChunks() const;

        // returns true if a voxel is present at the world position
        // returns false if the voxel is air or the chunk is not loaded
        [[nodiscard]] bool IsVoxelAt(glm::ivec3 worldPosition) const;

        // Get voxel at a position, returns empty if the chunk isn't loaded
        [[nodiscard]] VoxelType GetVoxelAt(glm::ivec3 worldPosition) const;

        // Set the voxel type if the chunk is loaded
        bool TrySetVoxelAt(glm::ivec3 worldPosition, VoxelType voxelType);

        // Convert from voxel coords to chunk coords
        // e.g. (67,13,-9) -> (1,0,-1)
        [[nodiscard]] static glm::ivec3 GetChunkPositionOfVoxel(glm::ivec3 voxelWorldPosition);

        // Convert chunk coords and voxel position in chunk (0 to 63 range) to world voxel coordinates
        [[nodiscard]] static glm::ivec3 GetWorldVoxelPositionInChunk(glm::ivec3 chunkPosition, glm::uvec3 voxelPositionInChunk);

        // Convert world voxel coords to chunk space (0 to 63 range)
        [[nodiscard]] static glm::uvec3 ToChunkSpace(glm::ivec3 worldVoxelPosition);

    private:
        void Update() const;

    private:
        // https://en.cppreference.com/w/cpp/container/unordered_map.html - always iterates in the same order if the map hasnt been changed
        std::unordered_map<glm::ivec3, std::unique_ptr<Chunk> > m_chunks;
        std::unique_ptr<VoxelWorldRenderer> m_renderer;
        std::unique_ptr<ProceduralGenerationManager> m_proceduralGenerationManager;
        Spire::Engine &m_engine;
        std::unique_ptr<LODManager> m_lodManager;
    };
} // SpireVoxel
