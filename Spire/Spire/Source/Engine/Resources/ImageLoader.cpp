#include "ImageLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Utils/Log.h"

namespace Spire
{
    int ImageLoader::s_numLoadedImages = 0;

    LoadedImage ImageLoader::LoadImageFromFile(const char *filename, ImageLoadSettings settings) {
        LoadedImage loadedImage = {};

        stbi_set_flip_vertically_on_load(settings.FlipVerticallyOnLoad);

        loadedImage.Data = stbi_load(filename, &loadedImage.Dimensions.x, &loadedImage.Dimensions.y, &loadedImage.NumChannels,
                                     STBI_rgb_alpha);

        if (loadedImage.Data) {
            s_numLoadedImages++;
        } else {
            error("Error loading texture from {}", filename);
        }

        return loadedImage;
    }

    void ImageLoader::UnloadImage(const LoadedImage &image) {
        assert(s_numLoadedImages > 0);
        assert(image.Data);
        stbi_image_free(image.Data);
        s_numLoadedImages--;
    }

    int ImageLoader::GetNumLoadedImages() {
        return s_numLoadedImages;
    }
}