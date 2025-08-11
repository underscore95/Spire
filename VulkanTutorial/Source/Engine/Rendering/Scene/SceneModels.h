#pragma once

#include <vector>
#include "Engine/Rendering/Memory/BufferManager.h"
#include "Engine/Rendering/Memory/VulkanBuffer.h"
#include "Engine/Resources/Model.h"

class GraphicsPipeline;
struct PipelineResourceInfo;
class RenderingManager;
struct Mesh;

class SceneModels
{
private:
    struct SceneMesh
    {
        glm::u32 NumVertices = 0;
        glm::u32 VertexStartIndex = 0;
        glm::u32 TextureIndex = 0;
    };

public:
    SceneModels(
        RenderingManager& renderingManager,
        const std::vector<Model>& models
    );
    ~SceneModels();

public:
    void CmdRenderModels(VkCommandBuffer commandBuffer,
                         const GraphicsPipeline& pipeline,
                         glm::u32 modelIndex,
                         glm::u32 instances = 1
    ) const;
    [[nodiscard]] std::array<PipelineResourceInfo, 1> GetPipelineResourceInfo() const;

private:
    void CreateVertexBuffer(const std::vector<Model>& models);

private:
    RenderingManager& m_renderingManager;
    VulkanBuffer m_vertexStorageBuffer;
    std::vector<std::vector<SceneMesh>> m_models;
};
