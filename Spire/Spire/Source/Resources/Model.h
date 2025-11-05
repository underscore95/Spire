#pragma once

#include <memory>
#include <vector>

namespace Spire
{
    struct Mesh;
    typedef std::vector<std::unique_ptr<Mesh>> Model;
}