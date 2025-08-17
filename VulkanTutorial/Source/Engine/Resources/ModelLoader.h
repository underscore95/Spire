#pragma once

#include <string>

#include "Model.h"

namespace Spire
{
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

        // imagePaths - list of images we're going to load, new ones may be added
        static Model LoadModel(const char* fileName, std::vector<std::string>& imagePaths, const ModelLoadingSettings& settings = {});
    };
}