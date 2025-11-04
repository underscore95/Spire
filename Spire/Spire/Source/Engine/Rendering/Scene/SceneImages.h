#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

namespace Spire
{
    struct VulkanImage;
    class RenderingManager;
    struct PipelineResourceInfo;
    struct Descriptor;

    class SceneImages
    {
    public:
        SceneImages(RenderingManager& renderingManager,
             const std::string& assetsDirectory,
             const std::vector<std::string>& imageFileNames);
        ~SceneImages();

    public:
        [[nodiscard]] Descriptor GetDescriptor(glm::u32 binding) const;

        glm::u32 NumLoadedImages() const;

    private:
        RenderingManager& m_renderingManager;
        std::vector<VulkanImage> m_loadedImages;
    };
}