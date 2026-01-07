#pragma once

#include "EngineIncludes.h"
#include "../../../Assets/Shaders/ShaderInfo.h"
#include "Meshers/ChunkMesher.h"

namespace SpireVoxel {
    struct Chunk;
}

namespace SpireVoxel {
    enum class MeshResult {
        Success,
        AlreadyInQueue,
        ChunkUnloaded
    };

    typedef std::function<void(std::vector<VertexData>, MeshResult)> Callback;

    class ChunkMesherManager {
        // Chunk queued for meshing
        struct QueuedChunk {
            std::shared_ptr<Chunk> Chunk;
            Callback Callback;
        };

        // Chunk that is currently being meshed
        struct ProcessingChunk {
            Callback Callback;
            std::future<std::vector<VertexData>> Future;
        };

    public:
        explicit ChunkMesherManager(Spire::RenderingManager &renderingManager);

    public:
        void Update();

        // Mesh a chunk, returns a possibly-null promise containing mesh
        // callback will be called on main thread once the mesh is generated
        void Mesh(const std::shared_ptr<Chunk> &chunk, const Callback &callback = {});

    private:
        std::unordered_map<std::shared_ptr<Chunk>, ProcessingChunk> m_currentlyMeshing;
        std::vector<QueuedChunk> m_queue;
        std::vector<std::unique_ptr<ChunkMesher> > m_meshers; // ordered so that lower meshers have higher priority (so they should be faster)
    };
} // SpireVoxel
