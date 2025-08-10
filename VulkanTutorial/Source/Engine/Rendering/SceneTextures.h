#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

struct VulkanImage;
class RenderingManager;
struct PipelineResourceInfo;

class SceneTextures
{
public:
    SceneTextures(RenderingManager& renderingManager, const std::vector<std::string>& textureFileNames);
    ~SceneTextures();

public:
    PipelineResourceInfo GetPipelineResourceInfo(glm::u32 binding);
    glm::u32 NumLoadedTextures() const;

private:
    RenderingManager& m_renderingManager;
    std::vector<VulkanImage> m_loadedTextures;
};
