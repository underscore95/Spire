#pragma once

#include <memory>
#include <vector>

#include "BufferManager.h"
#include "VulkanBuffer.h"

struct PipelineResourceInfo;
class RenderingManager;
struct Mesh;

typedef std::vector<std::unique_ptr<Mesh>> Model;

class SceneModels
{
private:
    struct SceneMesh
    {
        glm::u32 NumVertices = 0;
        glm::u32 GlobalMeshIndex = 0; // pretend m_models is a flat array what would our index be?
        glm::u32 VertexStartIndex = 0;
    };

public:
    SceneModels(RenderingManager& renderingManager, const std::vector<Model>& models);
    ~SceneModels();

public:
    void CmdRenderModels(VkCommandBuffer commandBuffer, glm::u32 modelIndex, glm::u32 currentImage,
                 glm::u32 instances = 1) const;
    [[nodiscard]] std::array<PipelineResourceInfo, 2> GetPipelineResourceInfo() const;

private:
    void CreateVertexBuffer(const std::vector<Model>& models);
    void CreateAndPopulateVertexOffsetStagingBuffer();

private:
    RenderingManager& m_renderingManager;
    VulkanBuffer m_vertexStorageBuffer;
    std::vector<VulkanBuffer> m_vertexOffsetBuffers;
    VulkanBuffer m_vertexOffsetStagingBuffer;
    std::vector<std::vector<SceneMesh>> m_models;
};
