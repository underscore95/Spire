#pragma once

#include <memory>
#include <vector>

struct Mesh;
typedef std::vector<std::unique_ptr<Mesh>> Model;
