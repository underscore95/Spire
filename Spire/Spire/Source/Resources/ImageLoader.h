#pragma once

#include "pch.h"

namespace Spire
{
    struct ImageLoadSettings {
        bool FlipVerticallyOnLoad = true;
    };

    struct LoadedImage {
        unsigned char *Data;
        glm::ivec2 Dimensions;
        int NumChannels;

        bool IsValid() const { return Data; }
    };

    class ImageLoader {
    public:
        ImageLoader() = delete;

        static LoadedImage LoadImageFromFile(const char *filename, ImageLoadSettings = {});

        static void UnloadImage(const LoadedImage &image);

        static int GetNumLoadedImages();

    private:
        static int s_numLoadedImages;
    };
}