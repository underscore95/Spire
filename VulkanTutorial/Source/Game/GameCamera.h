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
    explicit GameCamera(const Engine& engine);

    void Update();
    void Render(const RenderInfo& renderInfo);

    Camera& GetCamera() const;

    PerImageDescriptor GetDescriptor(glm::u32 binding) const;

private:
    const Engine& m_engine;
    std::unique_ptr<PerImageBuffer> m_uniformBuffer = nullptr;
    std::unique_ptr<Camera> m_camera;
    CameraInfo m_cameraInfo;
};
