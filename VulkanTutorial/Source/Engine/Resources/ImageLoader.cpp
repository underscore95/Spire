#include "ImageLoader.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <libassert/assert.hpp>
#include <spdlog/spdlog.h>

namespace Spire
{
    int ImageLoader::s_numLoadedImages = 0;

    LoadedImage ImageLoader::LoadImage(const char *filename, ImageLoadSettings settings) {
        LoadedImage loadedImage = {};

        stbi_set_flip_vertically_on_load(settings.FlipVerticallyOnLoad);

        loadedImage.Data = stbi_load(filename, &loadedImage.Dimensions.x, &loadedImage.Dimensions.y, &loadedImage.NumChannels,
                                     STBI_rgb_alpha);

        if (loadedImage.Data) {
            s_numLoadedImages++;
        } else {
            spdlog::error("Error loading texture from {}", filename);
        }

        return loadedImage;
    }

    void ImageLoader::UnloadImage(const LoadedImage &image) {
        ASSERT(s_numLoadedImages > 0);
        ASSERT(image.Data);
        stbi_image_free(image.Data);
        s_numLoadedImages--;
    }

    int ImageLoader::GetNumLoadedImages() {
        return s_numLoadedImages;
    }
}