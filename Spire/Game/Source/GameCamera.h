#pragma once

#include "../Assets/Shaders/ShaderInfo.h"
#include "EngineIncludes.h"
#include "Utils/IVoxelCamera.h"

class GameCamera : public SpireVoxel::IVoxelCamera {
public:
    explicit GameCamera(const Spire::Engine &engine);

    void Update();

    void Render(const SpireVoxel::RenderInfo &renderInfo) override;

    [[nodiscard]] Spire::Camera &GetCamera() const;

    [[nodiscard]] Spire::PerImageDescriptor GetDescriptor(glm::u32 binding) const override;

    [[nodiscard]] SpireVoxel::CameraInfo GetCameraInfo() const override;

private:
    const Spire::Engine &m_engine;
    std::unique_ptr<Spire::PerImageBuffer> m_uniformBuffer = nullptr;
    std::unique_ptr<Spire::Camera> m_camera;
    SpireVoxel::CameraInfo m_cameraInfo{};
};
