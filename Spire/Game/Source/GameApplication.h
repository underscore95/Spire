#pragma once

#include "VoxelRenderer.h"

class GameApplication final : public Spire::Application {
public:
    GameApplication();

    ~GameApplication() override;

public:
    void Start(Spire::Engine &engine) override;

    void Cleanup();

    void Update() override;

    void Render() override;

    void RenderUi() const;

    [[nodiscard]] bool ShouldClose() const override;

    [[nodiscard]] const char *GetApplicationName() const override;

    void OnWindowResize() const override;

private:
    Spire::Engine *m_engine;
    std::unique_ptr<SpireVoxel::VoxelRenderer> m_voxelRenderer;
    glm::u32 m_frame = 0;
    glm::u32 m_swapchainImageIndex = 0;
};
