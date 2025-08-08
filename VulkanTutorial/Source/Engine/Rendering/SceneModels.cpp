#include "SceneModels.h"
#include <libassert/assert.hpp>
#include "BufferManager.h"
#include "PipelineDescriptorSetsManager.h"
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

    m_vertexOffsetBuffers = m_renderingManager.GetBufferManager().CreateUniformBuffers(sizeof(VertexOffset), true);

    CreateAndPopulateVertexOffsetStagingBuffer();
}

SceneModels::~SceneModels()
{
    m_renderingManager.GetBufferManager().DestroyBuffer(m_vertexOffsetStagingBuffer);
    m_renderingManager.GetBufferManager().DestroyBuffer(m_vertexStorageBuffer);
    for (auto& buffer : m_vertexOffsetBuffers)
    {
        m_renderingManager.GetBufferManager().DestroyBuffer(buffer);
    }
}

void SceneModels::CmdRenderModels(VkCommandBuffer commandBuffer, glm::u32 modelIndex, glm::u32 currentImage,
                          glm::u32 instances) const
{
    DEBUG_ASSERT(modelIndex < m_models.size());
    for (glm::u32 meshIndex = 0; meshIndex < m_models[modelIndex].size(); meshIndex++)
    {
        auto& sceneMesh = m_models[modelIndex][meshIndex];

        // Copy global index into the vertex offset buffer (tells the shader where the vertices start)
        VkBufferCopy region = {
            .srcOffset = sceneMesh.GlobalMeshIndex * sizeof(glm::u32),
            .dstOffset = 0,
            .size = sizeof(glm::u32)
        };
        vkCmdCopyBuffer(
            commandBuffer,
            m_vertexOffsetStagingBuffer.Buffer,
            m_vertexOffsetBuffers[currentImage].Buffer,
            1, &region
        );

        // barrier
        VkBufferMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.buffer = m_vertexOffsetBuffers[currentImage].Buffer;
        barrier.offset = 0;
        barrier.size = VK_WHOLE_SIZE;

        vkCmdPipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
            0,
            0, nullptr,
            1, &barrier,
            0, nullptr
        );

        // draw the mesh
        m_renderingManager.GetRenderer().BeginDynamicRendering(commandBuffer, currentImage, nullptr, nullptr);
        vkCmdDraw(commandBuffer, sceneMesh.NumVertices, instances, 0, 0);
        vkCmdEndRendering(commandBuffer);
    }
}

std::array<PipelineResourceInfo, 2> SceneModels::GetPipelineResourceInfo() const
{
    std::array info = {
        PipelineResourceInfo{
            .ResourceType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .Binding = 0,
            .Stages = VK_SHADER_STAGE_VERTEX_BIT,
            .SameResourceForAllImages = true,
            .ResourcePtrs = &m_vertexStorageBuffer
        },

        PipelineResourceInfo{
            .ResourceType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .Binding = 1,
            .Stages = VK_SHADER_STAGE_VERTEX_BIT,
            .SameResourceForAllImages = false,
            .ResourcePtrs = m_vertexOffsetBuffers.data()
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
                .GlobalMeshIndex = numMeshes,
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

void SceneModels::CreateAndPopulateVertexOffsetStagingBuffer()
{
    // get data
    std::vector<glm::u32> vertexOffsets;
    for (auto& model : m_models)
    {
        for (auto& mesh : model)
        {
            vertexOffsets.push_back(mesh.VertexStartIndex);
        }
    }

    // create buffer
    m_vertexOffsetStagingBuffer = m_renderingManager.GetBufferManager().CreateStorageBuffer(
        vertexOffsets.data(), vertexOffsets.size() * sizeof(glm::u32), sizeof(glm::u32), true
    );

    // upload data
    m_renderingManager.GetBufferManager().UpdateBuffer(
        m_vertexOffsetStagingBuffer, vertexOffsets.data(),
        vertexOffsets.size() * sizeof(glm::u32)
    );
}
