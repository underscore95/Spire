#pragma once

#include <string>

#include "Model.h"

struct ModelLoadingSettings
{
    bool TriangulateFaces = true;
    bool FlipUVs = true;
    bool IgnoreNonTriangleMeshes = true;
};

class ModelLoader
{
public:
    ModelLoader() = delete;

    // texturePaths - list of textures we're going to load, new ones may be added
    static Model LoadModel(const char* fileName, std::vector<std::string>& texturePaths, const ModelLoadingSettings& settings = {});
};
