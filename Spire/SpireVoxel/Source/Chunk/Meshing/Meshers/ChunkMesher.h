#pragma once

#include "EngineIncludes.h"
#include "../../../../Assets/Shaders/ShaderInfo.h"

namespace SpireVoxel {
    struct Chunk;

    class ChunkMesher {
    public:
        virtual ~ChunkMesher() = default;

    public:
        // Mesh a chunk
        [[nodiscard]] virtual std::future<std::vector<VertexData> > Mesh(Chunk &chunk) = 0;

        // How much load is this mesher under?
        // Lower values mean the mesher is more likely to be assigned work
        // do not return negative numbers
        [[nodiscard]] virtual float GetLoad() = 0;

        // todo average mesh time + num parallel

    protected:
        // Push vertices to form a face
        // vertices - vector to push to
        // face - SPIRE_VOXEL_FACE_POS_X etc
        // p - position in chunk (0 to SPIRE_VOXEL_CHUNK_SIZE) - note that some vertices will be positioned at p+1 on some axises
        // width - width of face >= 1
        // height - height of face >= 1
        void PushFace(std::vector<VertexData> &vertices, glm::u32 face, glm::uvec3 p, glm::u32 width, glm::u32 height);
    };
} // SpireVoxel
