#pragma once

#include "Engine/EngineIncludes.h"

struct UniformData
{
    glm::mat4 WVP;
};

class GameApplication final : public Application
{
public:
    void Start(Engine& engine) override;

    ~GameApplication() override;

    void Update() override;

    void Render() override;

    [[nodiscard]] bool ShouldClose() const override;

    [[nodiscard]] std::string GetApplicationName() const override;

private:
    void BeginRendering(VkCommandBuffer commandBuffer, glm::u32 imageIndex) const;

    void RecordCommandBuffers() const;

    void CreateStorageBufferForVertices();

    void CreateUniformBuffers();

    void UpdateUniformBuffers(glm::u32 imageIndex) const;

    void SetupGraphicsPipeline();

private:
    Engine* m_engine = nullptr;
    std::vector<VkCommandBuffer> m_commandBuffers;
    VkShaderModule m_vertexShader = VK_NULL_HANDLE;
    VkShaderModule m_fragmentShader = VK_NULL_HANDLE;
    std::unique_ptr<GraphicsPipeline> m_graphicsPipeline = nullptr;
    VulkanBuffer m_vertexStorageBuffer = {};
    glm::u32 m_vertexBufferSize = 0;
    std::vector<VulkanBuffer> m_uniformBuffers;
    std::unique_ptr<Camera> m_camera;
    VulkanImage m_texture;
};
