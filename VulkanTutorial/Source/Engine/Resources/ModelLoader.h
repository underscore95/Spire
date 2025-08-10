#pragma once

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

    static Model LoadModel(const char* fileName, const ModelLoadingSettings& settings = {});
};
