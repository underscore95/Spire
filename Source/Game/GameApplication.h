#pragma once

#include "Engine/EngineIncludes.h"

class GameApplication final : public Application {
public:
    void Start(Engine &engine) override;

    ~GameApplication() override;

    void Update() override;

    void Render() override;

    [[nodiscard]] bool ShouldClose() const override;

    [[nodiscard]] std::string GetApplicationName() const override;

private:
    void RecordCommandBuffers() const;

    void CreateStorageBufferForVertices();

private:
    Engine *m_engine = nullptr;
    std::vector<VkCommandBuffer> m_commandBuffers;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> m_frameBuffers;
    VkShaderModule m_vertexShader = VK_NULL_HANDLE;
    VkShaderModule m_fragmentShader = VK_NULL_HANDLE;
    std::unique_ptr<GraphicsPipeline> m_graphicsPipeline = nullptr;
    VulkanBuffer m_vertexStorageBuffer = {};
    glm::u32 m_vertexBufferSize = 0;
};
