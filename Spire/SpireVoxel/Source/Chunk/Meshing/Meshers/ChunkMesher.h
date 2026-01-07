#pragma once

#include "EngineIncludes.h"
#include "../../../../Assets/Shaders/ShaderInfo.h"

namespace SpireVoxel {
    struct Chunk;

    // Multithreaded chunk mesher
    class ChunkMesher {
    public:
        virtual ~ChunkMesher() = default;

        void Init();

    public:
        // Call the internal mesh function, automatically unlocks the thread id after completion
        [[nodiscard]] std::vector<VertexData>  Mesh(const std::shared_ptr<Chunk> &chunk, glm::u32 threadId);

        // Lock a thread id so you can mesh a chunk
        // Returns an empty optional if this mesher cannot mesh another chunk at the moment
        [[nodiscard]] std::optional<glm::u32> LockThreadId();

        // How much load is this mesher under?
        // Lower values mean the mesher is more likely to be assigned work
        // negative value means mesher cannot accept new world
        [[nodiscard]] virtual float GetLoad() = 0;

        // Maximum number of chunks that can be meshed in parallel, may change on different systems however must return the same number each time the method is called until ChunkMesherManager is destructed
        [[nodiscard]] virtual glm::u32 GetMaxParallelisation() const = 0;

        [[nodiscard]] glm::u32 GetNumChunksProcessing() const;

        // todo average mesh time + num parallel

    protected:
        // Push vertices to form a face
        // You cannot call this in parallel on the same thread!
        // vertices - vector to push to
        // face - SPIRE_VOXEL_FACE_POS_X etc
        // p - position in chunk (0 to SPIRE_VOXEL_CHUNK_SIZE) - note that some vertices will be positioned at p+1 on some axises
        // width - width of face >= 1
        // height - height of face >= 1
        static void PushFace(std::vector<VertexData> &vertices, glm::u32 face, glm::uvec3 p, glm::u32 width, glm::u32 height);

        // Actually mesh the chunk
        [[nodiscard]] virtual std::vector<VertexData> GenerateMesh(const std::shared_ptr<Chunk> &chunk, glm::u32 threadId) = 0;

        // Called on init, while the mutex is locked, just before m_initialized is set to true
        virtual void OnInit() = 0;

    private:
        std::mutex m_mutex;
        glm::u32 m_numChunksProcessing = 0;
        std::vector<bool> m_threadIdLocks; // Each thread needs an id to mesh, true indices are locked and currently in use
        bool m_initialized = false;
    };
} // SpireVoxel
