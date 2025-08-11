#pragma once

#include <glm/glm.hpp>
#include <vector>

struct ModelVertex
{
public:
    ModelVertex(const glm::vec3& p, const glm::vec2& t)
    {
        Pos = p;
        Tex = t;
    }

    ModelVertex() = default;

public:
    glm::vec3 Pos;
    glm::vec2 Tex;
};

struct Mesh
{
public:
    Mesh() = default;

public:
    std::vector<ModelVertex> Vertices;
    glm::u32 TextureIndex;
};
