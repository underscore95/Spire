#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

struct VulkanImage;
class RenderingManager;
struct PipelineResourceInfo;
struct Descriptor;

class SceneTextures
{
public:
    SceneTextures(RenderingManager& renderingManager, const std::vector<std::string>& textureFileNames);
    ~SceneTextures();

public:
    [[nodiscard]] Descriptor GetDescriptor(glm::u32 binding) const;

    glm::u32 NumLoadedTextures() const;

private:
    RenderingManager& m_renderingManager;
    std::vector<VulkanImage> m_loadedTextures;
};
