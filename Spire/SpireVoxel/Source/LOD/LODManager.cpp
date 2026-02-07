#include "LODManager.h"

#include "ISamplingOffsets.h"
#include "Chunk/VoxelWorld.h"
#include "Rendering/VoxelWorldRenderer.h"

namespace SpireVoxel {
    void ReduceDetail(ISamplingOffsets &samplingOffsets, Chunk &reduceInto, const Chunk &target, glm::u32 newLODScale) {
        const bool same = &reduceInto == &target;

        std::unique_ptr<std::array<VoxelType, SPIRE_VOXEL_CHUNK_VOLUME> > srcData{};
        const std::array<VoxelType, SPIRE_VOXEL_CHUNK_VOLUME> *src = &target.VoxelData;

        if (same) {
            srcData = std::make_unique<std::array<VoxelType, SPIRE_VOXEL_CHUNK_VOLUME> >(reduceInto.VoxelData);
            src = srcData.get();
            reduceInto.VoxelData = {};
        }

        glm::uvec3 offset = static_cast<glm::vec3>(target.ChunkPosition - reduceInto.ChunkPosition) * static_cast<float>(SPIRE_VOXEL_CHUNK_SIZE / newLODScale);

        for (glm::u32 x = 0; x < SPIRE_VOXEL_CHUNK_SIZE / newLODScale; x++) {
            for (glm::u32 y = 0; y < SPIRE_VOXEL_CHUNK_SIZE / newLODScale; y++) {
                for (glm::u32 z = 0; z < SPIRE_VOXEL_CHUNK_SIZE / newLODScale; z++) {
                    auto sampleOffset = static_cast<glm::u32>(samplingOffsets.GetOffset(x, y, z) * static_cast<float>(newLODScale));
                    glm::u32 readIndex = SPIRE_VOXEL_POSITION_XYZ_TO_INDEX(
                        x * newLODScale + sampleOffset,
                        y * newLODScale + sampleOffset,
                        z * newLODScale + sampleOffset
                    );
                    assert(readIndex < SPIRE_VOXEL_CHUNK_VOLUME);
                    VoxelType type = (*src)[readIndex];

                    glm::u32 writeIndex = SPIRE_VOXEL_POSITION_XYZ_TO_INDEX(
                        x + offset.x,
                        y + offset.y,
                        z + offset.z
                    );
                    assert(writeIndex < SPIRE_VOXEL_CHUNK_VOLUME);
                    reduceInto.VoxelData[writeIndex] = type;
                }
            }
        }

        assert(!reduceInto.IsCorrupted());
    }

    LODManager::LODManager(
        VoxelWorld &world,
        const std::shared_ptr<ISamplingOffsets> &samplingOffsets
    )
        : m_world(world),
          m_samplingOffsets(samplingOffsets) {
    }

    void LODManager::IncreaseLODTo(Chunk &chunk, glm::u32 newLODScale) {
        constexpr bool PROFILING_LOD = false;
        if (chunk.LOD.Scale >= newLODScale) {
            Spire::error("Failed to increase LOD scale of chunk {} {} {} from {} to {} because new value is not higher", chunk.ChunkPosition.x,
                         chunk.ChunkPosition.y, chunk.ChunkPosition.z, chunk.LOD.Scale, newLODScale);
            assert(false);
            return;
        }

        assert(chunk.LOD.Scale == 1); // todo make support other scales?

        // Create a vector of all chunks that this chunk will now cover
        Spire::Timer timer;
        std::vector<Chunk *> coveredChunks;
        std::vector<glm::ivec3> coveredChunkPositions;
        coveredChunkPositions.reserve(newLODScale * newLODScale * newLODScale);
        coveredChunks.reserve(newLODScale * newLODScale * newLODScale);
        for (int x = chunk.ChunkPosition.x; x < chunk.ChunkPosition.x + static_cast<int>(newLODScale); x++) {
            for (int y = chunk.ChunkPosition.y; y < chunk.ChunkPosition.y + static_cast<int>(newLODScale); y++) {
                for (int z = chunk.ChunkPosition.z; z < chunk.ChunkPosition.z + static_cast<int>(newLODScale); z++) {
                    Chunk *coveredChunk = m_world.TryGetLoadedChunk({x, y, z});
                    if (coveredChunk && coveredChunk != &chunk) {
                        assert(coveredChunk->LOD.Scale == 1);
                        coveredChunks.push_back(coveredChunk);
                        coveredChunkPositions.push_back(coveredChunk->ChunkPosition);
                    }
                }
            }
        }
        if (PROFILING_LOD) Spire::info("Find chunks: {} ms", timer.MillisSinceStart());
        timer.Restart();

        // Reduce detail
        ReduceDetail(*m_samplingOffsets, chunk, chunk, newLODScale);

        if (PROFILING_LOD) Spire::info("Reduce detail of main chunk: {} ms", timer.MillisSinceStart());
        timer.Restart();

        // set everything in main chunk except squished main chunk voxels to air
        for (glm::u32 x = 0; x < SPIRE_VOXEL_CHUNK_SIZE; x++) {
            for (glm::u32 y = 0; y < SPIRE_VOXEL_CHUNK_SIZE; y++) {
                for (glm::u32 z = 0; z < SPIRE_VOXEL_CHUNK_SIZE; z++) {
                    if (x < SPIRE_VOXEL_CHUNK_SIZE / newLODScale && y < SPIRE_VOXEL_CHUNK_SIZE / newLODScale && z < SPIRE_VOXEL_CHUNK_SIZE / newLODScale) continue;
                    glm::u32 index = SPIRE_VOXEL_POSITION_TO_INDEX((glm::uvec3{x, y, z} ));
                    chunk.VoxelData[index] = 0;
                }
            }
        }

        if (PROFILING_LOD) Spire::info("Set to air: {} ms", timer.MillisSinceStart());
        timer.Restart();

        for (Chunk *coveredChunk : coveredChunks) {
            ReduceDetail(*m_samplingOffsets, chunk, *coveredChunk, newLODScale);
        }

        if (PROFILING_LOD) Spire::info("Reduce detail: {} ms", timer.MillisSinceStart());
        timer.Restart();

        chunk.LOD.Scale = newLODScale;
        m_world.UnloadChunks(coveredChunkPositions);

        if (PROFILING_LOD) Spire::info("Unload chunks: {} ms", timer.MillisSinceStart());
        timer.Restart();

        chunk.RegenerateVoxelBits();
        if (PROFILING_LOD) Spire::info("Regenerate chunks: {} ms", timer.MillisSinceStart());
        timer.Restart();
        m_world.GetRenderer().NotifyChunkEdited(chunk);

        if (PROFILING_LOD) Spire::info("Notify edit: {} ms", timer.MillisSinceStart());
        timer.Restart();
    }
} // SpireVoxel
