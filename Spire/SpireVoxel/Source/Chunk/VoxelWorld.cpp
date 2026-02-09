#include "VoxelWorld.h"
#include "Rendering/VoxelWorldRenderer.h"
#include "LOD/SamplingOffsets.h"

namespace SpireVoxel {
    VoxelWorld::VoxelWorld(
        Spire::Engine &engine,
        const std::shared_ptr<ISamplingOffsets> &samplingOffsets,
        const std::function<void()> &recreatePipelineCallback,
        bool isProfilingMeshing,
        std::unique_ptr<IProceduralGenerationProvider> provider,
        std::unique_ptr<IProceduralGenerationController> controller,
        IVoxelCamera &camera)
        : m_engine(engine) {
        m_renderer = std::make_unique<VoxelWorldRenderer>(*this, engine.GetRenderingManager(), recreatePipelineCallback, isProfilingMeshing);
        m_proceduralGenerationManager = std::make_unique<ProceduralGenerationManager>(std::move(provider), std::move(controller), *this, camera);
        m_lodManager = std::unique_ptr<LODManager>(new LODManager(*this, samplingOffsets));
    }

    LODManager &VoxelWorld::GetLODManager() const {
        return *m_lodManager;
    }

    ProceduralGenerationManager &VoxelWorld::GetProceduralGenerationManager() const {
        return *m_proceduralGenerationManager;
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

    void VoxelWorld::Update() const {
        m_proceduralGenerationManager->Update();
    }
} // SpireVoxel
