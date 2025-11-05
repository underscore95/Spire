#pragma once

#include "pch.h"
#include "Rendering/Memory/BufferManager.h"
#include "Rendering/Memory/VulkanBuffer.h"
#include "Resources/Model.h"

namespace Spire
{
    class GraphicsPipeline;
    struct Descriptor;
    class RenderingManager;
    struct Mesh;

    class SceneModels
    {
    private:
        struct SceneMesh
        {
            glm::u32 NumIndices = 0;
            glm::u32 FirstIndex = 0;
            glm::u32 ImageIndex = 0;
        };

    public:
        SceneModels(
            RenderingManager& renderingManager,
            const std::vector<Model>& models
        );
        ~SceneModels();

    public:
        void CmdBindIndexBuffer(
            VkCommandBuffer commandBuffer
            ) const;

        void CmdRenderModels(VkCommandBuffer commandBuffer,
                             const GraphicsPipeline& pipeline,
                             glm::u32 modelIndex,
                             glm::u32 instances = 1
        ) const;

        [[nodiscard]] Descriptor GetDescriptor(glm::u32 binding) const;

    private:
        void CreateVertexAndIndexBuffer(const std::vector<Model>& models);

    private:
        RenderingManager& m_renderingManager;
        VulkanBuffer m_vertexStorageBuffer;
        VulkanBuffer m_indexBuffer;
        std::vector<std::vector<SceneMesh>> m_models;
    };
}