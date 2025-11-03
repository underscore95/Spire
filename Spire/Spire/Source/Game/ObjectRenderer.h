#pragma once

#include "Engine/EngineIncludes.h"

namespace Spire
{
    class SceneModels;
}

class ObjectRenderer
{
public:
    ObjectRenderer(
        Spire::SceneModels& sceneModels,
        Spire::RenderingManager& renderingManager,
        glm::u32 modelIndex,
        glm::u32 modelDataSize,
        glm::u32 maxInstances
    );

    ~ObjectRenderer();

public:
    void SetModelData(glm::u32 imageIndex, glm::u32 index, const void* data);
    void SetModelDatas(glm::u32 imageIndex, glm::u32 startIndex, glm::u32 count, const void* data);
    void CmdRender(VkCommandBuffer commandBuffer, const Spire::GraphicsPipeline& pipeline, glm::u32 instances, bool bindIndexBuffer) const;

    Spire::PerImageDescriptor GetDescriptor() const;

private:
    Spire::RenderingManager& m_renderingManager;
    Spire::SceneModels& m_sceneModels;
    glm::u32 m_modelIndex;
    glm::u32 m_modelDataSize;
    glm::u32 m_maxInstances;
    std::unique_ptr<Spire::PerImageBuffer> m_modelDataBuffer;
};
