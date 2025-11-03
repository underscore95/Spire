#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace Spire
{
    struct ModelVertex
    {
    public:
        ModelVertex(const glm::vec3& pos, const glm::vec2& tex)
        {
            Pos = pos;
            Tex = tex;
        }

        ModelVertex() = default;

    public:
        glm::vec3 Pos;
        glm::vec2 Tex;
    };

    struct Mesh
    {
    public:
        std::vector<ModelVertex> Vertices;
        std::vector<glm::u32> Indices;
        glm::u32 ImageIndex;

#define MESH_INDEX_TYPE glm::u32
    };
}