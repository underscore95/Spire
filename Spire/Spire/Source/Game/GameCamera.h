#pragma once

#include "RenderInfo.h"
#include "Engine/EngineIncludes.h"

struct CameraInfo
{
    glm::mat4x4 ViewProjectionMatrix;
};

class GameCamera
{
public:
    explicit GameCamera(const Spire::Engine& engine);

    void Update();
    void Render(const RenderInfo& renderInfo);

    Spire::Camera& GetCamera() const;

    Spire::PerImageDescriptor GetDescriptor(glm::u32 binding) const;

private:
    const Spire::Engine& m_engine;
    std::unique_ptr<Spire::PerImageBuffer> m_uniformBuffer = nullptr;
    std::unique_ptr<Spire::Camera> m_camera;
    CameraInfo m_cameraInfo;
};
