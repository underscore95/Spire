#include "SceneModels.h"

#include "Rendering/Memory/BufferManager.h"
#include "Rendering/Core/Pipelines/GraphicsPipeline.h"
#include "PushConstants.h"
#include "Rendering/Renderers/Renderer.h"
#include "Rendering/RenderingManager.h"
#include "Rendering/Descriptors/Descriptor.h"
#include "Rendering/Memory/VulkanBuffer.h"
#include "Resources/Mesh.h"

namespace Spire
{
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
        CreateVertexAndIndexBuffer(models);
    }

    SceneModels::~SceneModels()
    {
        m_renderingManager.GetBufferManager().DestroyBuffer(m_indexBuffer);
        m_renderingManager.GetBufferManager().DestroyBuffer(m_vertexStorageBuffer);
    }

    void SceneModels::CmdBindIndexBuffer(VkCommandBuffer commandBuffer) const
    {
        static_assert(sizeof(MESH_INDEX_TYPE) == sizeof(glm::u32)); // update index type if changed!
        vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer.Buffer, 0, VK_INDEX_TYPE_UINT32);
    }

    void SceneModels::CmdRenderModels(
        VkCommandBuffer commandBuffer,
        const GraphicsPipeline& pipeline,
        glm::u32 modelIndex,
        glm::u32 instances
    ) const
    {
        assert(modelIndex < m_models.size());
        for (glm::u32 meshIndex = 0; meshIndex < m_models[modelIndex].size(); meshIndex++)
        {
            auto& sceneMesh = m_models[modelIndex][meshIndex];

            // set the texture index
            pipeline.CmdSetPushConstants(
                commandBuffer,
                &sceneMesh.ImageIndex,
                sizeof(sceneMesh.ImageIndex),
                static_cast<glm::u32>(offsetof(PushConstants, ImageIndex))
            );

            // draw the mesh
            vkCmdDrawIndexed(commandBuffer, sceneMesh.NumIndices, instances, sceneMesh.FirstIndex, 0, 0);
        }
    }

    Descriptor SceneModels::GetDescriptor(glm::u32 binding) const
    {
        return {
            .ResourceType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .Binding = binding,
            .Stages = VK_SHADER_STAGE_VERTEX_BIT,
            .Resources = {{.Buffer = &m_vertexStorageBuffer}}
        };
    }

    void SceneModels::CreateVertexAndIndexBuffer(const std::vector<Model>& models)
    {
        // calculate total size of all models
        constexpr glm::u32 VERTEX_SIZE = sizeof(ModelVertex);
        glm::u32 totalSize = 0;
        glm::u32 numMeshes = 0;
        glm::u32 numIndices = 0;
        for (auto& model : models)
        {
            m_models.push_back({});
            for (auto& mesh : model)
            {
                m_models.back().push_back({
                    .NumIndices = static_cast<glm::u32>(mesh->Indices.size()),
                    .FirstIndex = numIndices,
                    .ImageIndex = mesh->ImageIndex
                });
                totalSize += mesh->Vertices.size() * VERTEX_SIZE;
                numIndices += mesh->Indices.size();
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

        // create global index vector
        std::vector<MESH_INDEX_TYPE> indices;
        indices.reserve(numIndices);
        glm::u32 startingIndex = 0;
        for (auto& model : models)
        {
            for (auto& mesh : model)
            {
                for (glm::u32 index : mesh->Indices)
                {
                    indices.push_back(index + startingIndex);
                }
                startingIndex += mesh->Indices.size();
            }
        }

        // create index buffer
        m_indexBuffer = m_renderingManager.GetBufferManager().CreateIndexBuffer(
            sizeof(MESH_INDEX_TYPE), indices.data(), numIndices
        );
    }
}