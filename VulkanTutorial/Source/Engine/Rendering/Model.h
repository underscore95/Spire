#pragma once

#include <memory>
#include <vector>

#include "BufferManager.h"

struct PipelineResourceInfo;
struct VulkanBuffer;
class RenderingManager;
struct Mesh;

class Model
{
public:
    Model(RenderingManager& renderingManager, const std::vector<std::unique_ptr<Mesh>>& meshes);
    ~Model();

public:
    void CmdDraw(VkCommandBuffer commandBuffer, glm::u32 instances = 1) const;
    [[nodiscard]] std::vector<PipelineResourceInfo> GetPipelineResourceInfo() const;

private:
    void CreateVertexBuffers(const std::vector<std::unique_ptr<Mesh>>& meshes);
    void FreeVertexBuffers();

private:
    RenderingManager& m_renderingManager;
    std::vector<VulkanBuffer> m_vertexStorageBuffers;
};
