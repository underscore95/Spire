#include "GameCamera.h"

using namespace Spire;
using namespace SpireVoxel;

GameCamera::GameCamera(const Engine &engine)
    : m_engine(engine) {
    m_uniformBuffer = engine.GetRenderingManager().GetBufferManager().CreatePerImageUniformBuffers(sizeof(CameraInfo));
    m_camera = std::make_unique<Camera>(engine.GetWindow(), glm::vec3{0, 0, 10});
}

void GameCamera::Update() {
    m_camera->Update(m_engine.GetDeltaTime());
    m_cameraInfo.ViewProjectionMatrix = m_camera->GetProjectionMatrix() * m_camera->GetViewMatrix();
}

void GameCamera::Render(const RenderInfo &renderInfo) {
    m_uniformBuffer->Update(renderInfo.ImageIndex, &m_cameraInfo, sizeof(CameraInfo));
}

Camera &GameCamera::GetCamera() const {
    return *m_camera;
}

PerImageDescriptor GameCamera::GetDescriptor(glm::u32 binding) const {
    return m_engine.GetRenderingManager().GetDescriptorCreator().CreatePerImageUniformBuffer(
        binding, *m_uniformBuffer, VK_SHADER_STAGE_VERTEX_BIT
    );
}

CameraInfo GameCamera::GetCameraInfo() const {
    return m_cameraInfo;
}

glm::vec3 GameCamera::GetPosition() const {
    return m_camera->GetPosition();
}
