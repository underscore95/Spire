#include "VoxelWorld.h"
#include "Rendering/VoxelWorldRenderer.h"

namespace SpireVoxel {
    VoxelWorld::VoxelWorld(
        Spire::Engine &engine,
        const std::function<void()> &recreatePipelineCallback,
        bool isProfilingMeshing,
        std::unique_ptr<IProceduralGenerationProvider> provider,
        std::unique_ptr<IProceduralGenerationController> controller,
        IVoxelCamera &camera)
        : m_engine(engine) {
        m_renderer = std::make_unique<VoxelWorldRenderer>(*this, engine.GetRenderingManager(), recreatePipelineCallback, isProfilingMeshing);
        m_proceduralGenerationManager = std::make_unique<ProceduralGenerationManager>(std::move(provider), std::move(controller), *this, camera);
    }

    Chunk &VoxelWorld::LoadChunk(glm::ivec3 chunkPosition) {
        auto it = m_chunks.find(chunkPosition);
        if (it != m_chunks.end()) return *it->second;

        m_chunks.try_emplace(chunkPosition, std::make_unique<Chunk>(chunkPosition, *this));
        m_renderer->NotifyChunkLoadedOrUnloaded();
        return *m_chunks.find(chunkPosition)->second;
    }

    void VoxelWorld::LoadChunks(const std::vector<glm::ivec3> &chunkPositions) {
        bool loadedAnyChunks = false;

        for (auto chunkPosition : chunkPositions) {
            if (m_chunks.contains(chunkPosition)) continue;
            if (m_chunks.size() + 1 > VoxelWorldRenderer::MAXIMUM_LOADED_CHUNKS) break;
            m_chunks.try_emplace(chunkPosition, new Chunk(chunkPosition, *this)); // this constructs a unique ptr
            auto &chunk = m_chunks.at(chunkPosition);
            assert(!chunk->IsCorrupted());
            loadedAnyChunks = true;
        }

        if (loadedAnyChunks) {
            m_renderer->NotifyChunkLoadedOrUnloaded();
        }
    }

    void VoxelWorld::UnloadChunks(const std::vector<glm::ivec3> &chunkPositions) {
        bool unloadedAnyChunks = false;
        for (auto chunkPosition : chunkPositions) {
            auto it = m_chunks.find(chunkPosition);
            if (it == m_chunks.end()) continue;
            m_renderer->FreeChunkVertexBuffer(*it->second);
            m_renderer->FreeChunkVoxelDataBuffer(*it->second);
            m_chunks.erase(it);
            unloadedAnyChunks = true;
        }

        if (unloadedAnyChunks) {
            m_renderer->NotifyChunkLoadedOrUnloaded();
        }
    }

    Chunk *VoxelWorld::TryGetLoadedChunk(glm::ivec3 chunkPosition) {
        auto it = m_chunks.find(chunkPosition);
        return it != m_chunks.end() ? it->second.get() : nullptr;
    }

    const Chunk *VoxelWorld::TryGetLoadedChunk(glm::ivec3 chunkPosition) const {
        auto it = m_chunks.find(chunkPosition);
        return it != m_chunks.end() ? it->second.get() : nullptr;
    }

    bool VoxelWorld::IsLoaded(const Chunk &chunk) {
        Chunk *loaded = TryGetLoadedChunk(chunk.ChunkPosition);
        if (!loaded) return false;
        assert(loaded == &chunk); // make sure there isn't two chunks at the same position
        return true;
    }

    std::size_t VoxelWorld::NumLoadedChunks() const {
        return m_chunks.size();
    }

    std::unordered_map<glm::ivec3, std::unique_ptr<Chunk> >::iterator VoxelWorld::begin() {
        return m_chunks.begin();
    }

    std::unordered_map<glm::ivec3, std::unique_ptr<Chunk> >::iterator VoxelWorld::end() {
        return m_chunks.end();
    }

    void VoxelWorld::UnloadAllChunks() {
        std::vector<glm::ivec3> loadedChunks;
        loadedChunks.reserve(NumLoadedChunks());
        for (auto &[chunkPos, _] : m_chunks) {
            loadedChunks.push_back(chunkPos);
        }

        UnloadChunks(loadedChunks);
    }

    VoxelWorldRenderer &VoxelWorld::GetRenderer() const {
        return *m_renderer;
    }

    glm::u64 VoxelWorld::CalculateGPUMemoryUsageForChunks() const {
        return m_renderer->m_chunkVertexBufferAllocator.CalculateAllocatedOrPendingMemory() + m_renderer->m_chunkVoxelDataBufferAllocator.CalculateAllocatedOrPendingMemory();
    }

    glm::u64 VoxelWorld::CalculateCPUMemoryUsageForChunks() const {
        glm::u64 usage = 0;
        for (auto &pair : m_chunks) {
            usage += sizeof(*pair.second);
        }
        return usage;
    }

    bool VoxelWorld::IsVoxelAt(glm::ivec3 worldPosition) const {
        return GetVoxelAt(worldPosition) != VOXEL_TYPE_AIR;
    }

    VoxelType VoxelWorld::GetVoxelAt(glm::ivec3 worldPosition) const {
        glm::ivec3 chunkPos = GetChunkPositionOfVoxel(worldPosition);
        glm::ivec3 positionInChunk = worldPosition - chunkPos * SPIRE_VOXEL_CHUNK_SIZE;

        const Chunk *chunk = TryGetLoadedChunk(chunkPos);
        return chunk ? chunk->VoxelData[SPIRE_VOXEL_POSITION_TO_INDEX(positionInChunk)] : VOXEL_TYPE_AIR;
    }

    bool VoxelWorld::TrySetVoxelAt(glm::ivec3 worldPosition, VoxelType voxelType) {
        glm::ivec3 chunkPos = GetChunkPositionOfVoxel(worldPosition);
        glm::ivec3 positionInChunk = worldPosition - chunkPos * SPIRE_VOXEL_CHUNK_SIZE;

        Chunk *chunk = TryGetLoadedChunk(chunkPos);
        if (chunk) {
            chunk->SetVoxel(SPIRE_VOXEL_POSITION_TO_INDEX(positionInChunk), voxelType);
            m_renderer->NotifyChunkEdited(*chunk);
        }
        return chunk;
    }

    glm::ivec3 VoxelWorld::GetChunkPositionOfVoxel(glm::ivec3 voxelWorldPosition) {
        return {
            glm::floor(static_cast<float>(voxelWorldPosition.x) / static_cast<float>(SPIRE_VOXEL_CHUNK_SIZE)),
            glm::floor(static_cast<float>(voxelWorldPosition.y) / static_cast<float>(SPIRE_VOXEL_CHUNK_SIZE)),
            glm::floor(static_cast<float>(voxelWorldPosition.z) / static_cast<float>(SPIRE_VOXEL_CHUNK_SIZE))
        };
    }

    glm::ivec3 VoxelWorld::GetWorldVoxelPositionInChunk(glm::ivec3 chunkPosition, glm::uvec3 voxelPositionInChunk) {
        assert(voxelPositionInChunk.x < SPIRE_VOXEL_CHUNK_SIZE);
        assert(voxelPositionInChunk.y < SPIRE_VOXEL_CHUNK_SIZE);
        assert(voxelPositionInChunk.z < SPIRE_VOXEL_CHUNK_SIZE);
        return chunkPosition * SPIRE_VOXEL_CHUNK_SIZE + glm::ivec3(voxelPositionInChunk);
    }

    glm::uvec3 VoxelWorld::ToChunkSpace(glm::ivec3 worldVoxelPosition) {
        return glm::uvec3{
            abs(worldVoxelPosition.x % SPIRE_VOXEL_CHUNK_SIZE),
            abs(worldVoxelPosition.y % SPIRE_VOXEL_CHUNK_SIZE),
            abs(worldVoxelPosition.z % SPIRE_VOXEL_CHUNK_SIZE)
        };
    }

    void ReduceDetail(Spire::Random &random, Chunk &reduceInto, const Chunk &target, glm::u32 newLODScale) {
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
                    VoxelType newType = (*src)[SPIRE_VOXEL_POSITION_XYZ_TO_INDEX(
                        x * newLODScale + random.RandomInt(0,newLODScale),
                        y * newLODScale + random.RandomInt(0,newLODScale),
                        z * newLODScale + random.RandomInt(0,newLODScale)
                    )];

                    reduceInto.VoxelData[
                        SPIRE_VOXEL_POSITION_XYZ_TO_INDEX(
                            x + offset.x,
                            y + offset.y,
                            z + offset.z
                        )
                    ] = newType;
                }
            }
        }

        assert(!reduceInto.IsCorrupted());
    }

    void VoxelWorld::IncreaseLODTo(Chunk &chunk, glm::u32 newLODScale) {
        if (chunk.LOD.Scale >= newLODScale) {
            Spire::error("Failed to increase LOD scale of chunk {} {} {} from {} to {} because new value is not higher", chunk.ChunkPosition.x,
                         chunk.ChunkPosition.y, chunk.ChunkPosition.z, chunk.LOD.Scale, newLODScale);
            assert(false);
            return;
        }

        assert(chunk.LOD.Scale == 1); // todo make support other scales?

        // Create a vector of all chunks that this chunk will now cover
        std::vector<Chunk *> coveredChunks;
        std::vector<glm::ivec3> coveredChunkPositions;
        coveredChunkPositions.reserve(newLODScale * newLODScale * newLODScale);
        coveredChunks.reserve(newLODScale * newLODScale * newLODScale);
        for (int x = chunk.ChunkPosition.x; x < chunk.ChunkPosition.x + static_cast<int>(newLODScale); x++) {
            for (int y = chunk.ChunkPosition.y; y < chunk.ChunkPosition.y + static_cast<int>(newLODScale); y++) {
                for (int z = chunk.ChunkPosition.z; z < chunk.ChunkPosition.z + static_cast<int>(newLODScale); z++) {
                    Chunk *coveredChunk = TryGetLoadedChunk({x, y, z});
                    if (coveredChunk && coveredChunk != &chunk) {
                        assert(coveredChunk->LOD.Scale == 1);
                        coveredChunks.push_back(coveredChunk);
                        coveredChunkPositions.push_back(coveredChunk->ChunkPosition);
                    }
                }
            }
        }

        // Reduce detail
        ReduceDetail(m_engine.GetRandom(), chunk, chunk, newLODScale);

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

        for (Chunk *coveredChunk : coveredChunks) {
            ReduceDetail(m_engine.GetRandom(), chunk, *coveredChunk, newLODScale);
        }

        chunk.LOD.Scale = newLODScale;
        UnloadChunks(coveredChunkPositions);

        chunk.RegenerateVoxelBits();
        GetRenderer().NotifyChunkEdited(chunk);
    }

    void VoxelWorld::Update() const {
        m_proceduralGenerationManager->Update();
    }
} // SpireVoxel
