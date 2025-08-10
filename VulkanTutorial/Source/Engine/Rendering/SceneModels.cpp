#include "SceneModels.h"
#include <libassert/assert.hpp>
#include "BufferManager.h"
#include "GraphicsPipeline.h"
#include "PipelineDescriptorSetsManager.h"
#include "PushConstants.h"
#include "Renderer.h"
#include "RenderingManager.h"
#include "VulkanBuffer.h"
#include "Engine/Resources/Mesh.h"

struct VertexOffset
{
    glm::u32 Offset = 0;
};

SceneModels::SceneModels(
    RenderingManager& renderingManager,
    const std::vector<Model>& models
)
    : m_renderingManager(renderingManager)
{
    CreateVertexBuffer(models);
}

SceneModels::~SceneModels()
{
    m_renderingManager.GetBufferManager().DestroyBuffer(m_vertexStorageBuffer);
}

void SceneModels::CmdRenderModels(
    VkCommandBuffer commandBuffer,
    const GraphicsPipeline& pipeline,
    glm::u32 modelIndex,
    glm::u32 instances
) const
{
    DEBUG_ASSERT(modelIndex < m_models.size());
    for (glm::u32 meshIndex = 0; meshIndex < m_models[modelIndex].size(); meshIndex++)
    {
        auto& sceneMesh = m_models[modelIndex][meshIndex];

        // set starting vertex index
        pipeline.CmdSetPushConstants(
            commandBuffer,
            &sceneMesh.VertexStartIndex,
            sizeof(sceneMesh.VertexStartIndex),
            static_cast<glm::u32>(offsetof(PushConstants, StartingVertexIndex))
        );

        // draw the mesh
        vkCmdDraw(commandBuffer, sceneMesh.NumVertices, instances, 0, 0);
    }
}

std::array<PipelineResourceInfo, 1> SceneModels::GetPipelineResourceInfo() const
{
    std::array info = {
        PipelineResourceInfo{
            .ResourceType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .Binding = 0,
            .Stages = VK_SHADER_STAGE_VERTEX_BIT,
            .SameResourceForAllFrames = true,
            .ResourcePtrs = &m_vertexStorageBuffer
        }
    };

    return info;
}

void SceneModels::CreateVertexBuffer(const std::vector<Model>& models)
{
    // calculate total size of all models
    constexpr glm::u32 VERTEX_SIZE = sizeof(ModelVertex);
    glm::u32 totalSize = 0;
    glm::u32 numMeshes = 0;
    for (auto& model : models)
    {
        m_models.push_back({});
        for (auto& mesh : model)
        {
            m_models.back().push_back({
                .NumVertices = static_cast<glm::u32>(mesh->Vertices.size()),
                .VertexStartIndex = totalSize / VERTEX_SIZE
            });
            totalSize += mesh->Vertices.size() * VERTEX_SIZE;
            numMeshes++;
        }
    }

    // create vector with all the vertices
    std::vector<ModelVertex> vertices;
    vertices.reserve(totalSize / VERTEX_SIZE);
    for (auto& model : models)
    {
        for (auto& mesh : model)
        {
            for (auto& vertex : mesh->Vertices)
            {
                vertices.push_back(vertex);
            }
        }
    }

    // create buffer for vertices
    m_vertexStorageBuffer = m_renderingManager.GetBufferManager().CreateStorageBuffer(
        vertices.data(), totalSize, VERTEX_SIZE);
}
