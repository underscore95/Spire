#pragma once

#include "Engine/EngineIncludes.h"
#include "RenderInfo.h"

class GameCamera;

class GameApplication final : public Spire::Application
{
public:
    GameApplication();

    ~GameApplication() override;

public:
    void Start(Spire::Engine& engine) override;

    void Cleanup();

    void Update() override;

    void Render() override;

    void RenderUi() const;

    [[nodiscard]] bool ShouldClose() const override;

    [[nodiscard]] const char* GetApplicationName() const override;

    void OnWindowResize() const override;

private:
    void BeginRendering(VkCommandBuffer commandBuffer, glm::u32 imageIndex) const;

    void RecordCommandBuffers() const;

    // returns paths to textures to load
    std::vector<std::string> CreateModels();

    void SetupDescriptors();

    void SetupGraphicsPipeline();

private:
    Spire::Engine* m_engine = nullptr;
    std::vector<VkCommandBuffer> m_commandBuffers;
    VkShaderModule m_vertexShader = VK_NULL_HANDLE;
    VkShaderModule m_fragmentShader = VK_NULL_HANDLE;
    std::unique_ptr<Spire::GraphicsPipeline> m_graphicsPipeline = nullptr;
    std::unique_ptr<Spire::SceneModels> m_models = nullptr;
    std::unique_ptr<Spire::SceneTextures> m_sceneTextures = nullptr;
    std::unique_ptr<Spire::DescriptorManager> m_descriptorManager = nullptr;
    std::unique_ptr<GameCamera> m_camera = nullptr;
};
