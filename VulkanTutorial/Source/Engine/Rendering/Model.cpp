#include "Model.h"

#include <libassert/assert.hpp>

#include "BufferManager.h"
#include "PipelineDescriptorSetsManager.h"
#include "RenderingManager.h"
#include "VulkanBuffer.h"
#include "Engine/Resources/Mesh.h"

Model::Model(
    RenderingManager& renderingManager,
    const std::vector<std::unique_ptr<Mesh>>& meshes
)
    : m_renderingManager(renderingManager)
{
    CreateVertexBuffers(meshes);
}

Model::~Model()
{
    FreeVertexBuffers();
}

void Model::CmdDraw(VkCommandBuffer commandBuffer, glm::u32 instances) const
{
    for (auto& buffer : m_vertexStorageBuffers)
    {
        vkCmdDraw(commandBuffer, buffer.Count, instances, 0, 0);
    }
}

std::vector<PipelineResourceInfo> Model::GetPipelineResourceInfo() const
{
    std::vector<PipelineResourceInfo> info;
    info.reserve(m_vertexStorageBuffers.size());
    for (auto& buffer : m_vertexStorageBuffers)
    {
        info.push_back(PipelineResourceInfo{
            .ResourceType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .Binding = 0, // TODO
            .Stages = VK_SHADER_STAGE_VERTEX_BIT,
            .SameResourceForAllImages = true,
            .ResourcePtrs = &buffer
        });
    }
    ASSERT(info.size()==1);
    return info;
}

void Model::CreateVertexBuffers(const std::vector<std::unique_ptr<Mesh>>& meshes)
{
    m_vertexStorageBuffers.reserve(meshes.size());
    for (const std::unique_ptr<Mesh>& mesh : meshes)
    {
        glm::u32 elementSize = sizeof(ModelVertex);
        glm::u32 bufferSize = elementSize * mesh->m_vertices.size();
        VulkanBuffer vertexBuffer = m_renderingManager.GetBufferManager().CreateStorageBufferForVertices(
            mesh->m_vertices.data(), bufferSize, elementSize
        );
        m_vertexStorageBuffers.push_back(vertexBuffer);
    }
}

void Model::FreeVertexBuffers()
{
    for (auto& buffer : m_vertexStorageBuffers)
    {
        m_renderingManager.GetBufferManager().DestroyBuffer(buffer);
    }
    m_vertexStorageBuffers.clear();
}
