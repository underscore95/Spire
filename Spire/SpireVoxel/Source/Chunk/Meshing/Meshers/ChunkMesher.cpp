#include "ChunkMesher.h"

namespace SpireVoxel {
    void ChunkMesher::Init() {
        std::unique_lock<std::mutex> lock;
        m_threadIdLocks.resize(GetMaxParallelisation(), false);
        OnInit();
        m_initialized = true;
    }

    std::vector<VertexData> ChunkMesher::Mesh(const std::shared_ptr<Chunk> &chunk, glm::u32 threadId) {
        assert(m_initialized);
        assert(m_numChunksProcessing + 1 <= GetMaxParallelisation());
        assert(threadId < GetMaxParallelisation());
        std::unique_lock<std::mutex> lock;
        m_numChunksProcessing++;
        std::vector<VertexData> mesh = GenerateMesh(chunk, threadId);
        m_threadIdLocks[threadId] = false;
        m_numChunksProcessing--;
        return std::move(mesh);
    }

    glm::u32 ChunkMesher::GetNumChunksProcessing() const {
        std::unique_lock<std::mutex> lock;
        return m_numChunksProcessing;
    }

    void ChunkMesher::PushFace(std::vector<VertexData> &vertices, glm::u32 face, glm::uvec3 p, glm::u32 width, glm::u32 height) {
        assert(width > 0);
        assert(height > 0);
        const glm::u32 w = width;
        const glm::u32 h = height;
        switch (face) {
            case SPIRE_VOXEL_FACE_POS_Z:
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + 0, p.z + 1, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_POS_Z));
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + h, p.z + 1, VoxelVertexPosition::THREE, SPIRE_VOXEL_FACE_POS_Z));
                vertices.push_back(PackVertexData(w, h, p.x + w, p.y + h, p.z + 1, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_POS_Z));
                vertices.push_back(PackVertexData(w, h, p.x + w, p.y + h, p.z + 1, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_POS_Z));
                vertices.push_back(PackVertexData(w, h, p.x + w, p.y + 0, p.z + 1, VoxelVertexPosition::ONE, SPIRE_VOXEL_FACE_POS_Z));
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + 0, p.z + 1, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_POS_Z));
                break;

            case SPIRE_VOXEL_FACE_NEG_Z:
                vertices.push_back(PackVertexData(w, h, p.x + w, p.y + 0, p.z + 0, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_NEG_Z));
                vertices.push_back(PackVertexData(w, h, p.x + w, p.y + h, p.z + 0, VoxelVertexPosition::THREE, SPIRE_VOXEL_FACE_NEG_Z));
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + h, p.z + 0, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_NEG_Z));
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + h, p.z + 0, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_NEG_Z));
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + 0, p.z + 0, VoxelVertexPosition::ONE, SPIRE_VOXEL_FACE_NEG_Z));
                vertices.push_back(PackVertexData(w, h, p.x + w, p.y + 0, p.z + 0, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_NEG_Z));
                break;

            case SPIRE_VOXEL_FACE_NEG_X:
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + 0, p.z + 0, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_NEG_X));
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + h, p.z + 0, VoxelVertexPosition::THREE, SPIRE_VOXEL_FACE_NEG_X));
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + h, p.z + w, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_NEG_X));
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + h, p.z + w, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_NEG_X));
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + 0, p.z + w, VoxelVertexPosition::ONE, SPIRE_VOXEL_FACE_NEG_X));
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + 0, p.z + 0, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_NEG_X));
                break;

            case SPIRE_VOXEL_FACE_POS_X:
                vertices.push_back(PackVertexData(w, h, p.x + 1, p.y + 0, p.z + w, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_POS_X));
                vertices.push_back(PackVertexData(w, h, p.x + 1, p.y + h, p.z + w, VoxelVertexPosition::THREE, SPIRE_VOXEL_FACE_POS_X));
                vertices.push_back(PackVertexData(w, h, p.x + 1, p.y + h, p.z + 0, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_POS_X));
                vertices.push_back(PackVertexData(w, h, p.x + 1, p.y + h, p.z + 0, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_POS_X));
                vertices.push_back(PackVertexData(w, h, p.x + 1, p.y + 0, p.z + 0, VoxelVertexPosition::ONE, SPIRE_VOXEL_FACE_POS_X));
                vertices.push_back(PackVertexData(w, h, p.x + 1, p.y + 0, p.z + w, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_POS_X));
                break;

            case SPIRE_VOXEL_FACE_POS_Y:
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + 1, p.z + h, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_POS_Y));
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + 1, p.z + 0, VoxelVertexPosition::THREE, SPIRE_VOXEL_FACE_POS_Y));
                vertices.push_back(PackVertexData(w, h, p.x + w, p.y + 1, p.z + 0, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_POS_Y));
                vertices.push_back(PackVertexData(w, h, p.x + w, p.y + 1, p.z + 0, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_POS_Y));
                vertices.push_back(PackVertexData(w, h, p.x + w, p.y + 1, p.z + h, VoxelVertexPosition::ONE, SPIRE_VOXEL_FACE_POS_Y));
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + 1, p.z + h, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_POS_Y));
                break;

            case SPIRE_VOXEL_FACE_NEG_Y:
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + 0, p.z + 0, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_NEG_Y));
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + 0, p.z + h, VoxelVertexPosition::THREE, SPIRE_VOXEL_FACE_NEG_Y));
                vertices.push_back(PackVertexData(w, h, p.x + w, p.y + 0, p.z + h, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_NEG_Y));
                vertices.push_back(PackVertexData(w, h, p.x + w, p.y + 0, p.z + h, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_NEG_Y));
                vertices.push_back(PackVertexData(w, h, p.x + w, p.y + 0, p.z + 0, VoxelVertexPosition::ONE, SPIRE_VOXEL_FACE_NEG_Y));
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + 0, p.z + 0, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_NEG_Y));
                break;
            default:
                assert(false);
                break;
        }
    }

    std::optional<glm::u32> ChunkMesher::LockThreadId() {
        std::unique_lock<std::mutex> lock;
        for (glm::u32 i = 0; i < m_threadIdLocks.size(); i++) {
            if (!m_threadIdLocks[i]) {
                m_threadIdLocks[i] = true; // lock it
                return i;
            }
        }
        return std::nullopt;
    }
} // SpireVoxel
