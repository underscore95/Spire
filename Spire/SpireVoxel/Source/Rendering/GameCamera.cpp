#include "GameCamera.h"

#include "Chunk/VoxelWorld.h"
using namespace Spire;

namespace SpireVoxel {
    GameCamera::GameCamera(const Engine &engine, VoxelWorld &world)
        : m_engine(engine),
          m_world(world) {
        m_uniformBuffer = engine.GetRenderingManager().GetBufferManager().CreateUniformBuffers(sizeof(CameraInfo));
        m_camera = std::make_unique<Camera>(engine.GetWindow(), glm::vec3{0, 0, 10});
        UpdateTargetedVoxel();
    }

    void GameCamera::Update() {
        m_camera->Update(m_engine.GetDeltaTime());
        UpdateTargetedVoxel();
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

    void GameCamera::SetTargetedVoxelRange(glm::u32 range) {
        m_targetedVoxelRange = range;
        UpdateTargetedVoxel();
    }

    const CameraInfo &GameCamera::GetCameraInfo() const {
        return m_cameraInfo;
    }

    std::optional<glm::ivec3> GameCamera::GetTargetedVoxelPosition() const {
        return m_cameraInfo.IsTargetingVoxel ? std::optional{glm::ivec3{m_cameraInfo.TargetedVoxelX, m_cameraInfo.TargetedVoxelY, m_cameraInfo.TargetedVoxelZ}} : std::nullopt;
    }

    std::optional<glm::ivec3> GameCamera::GetTargetedAdjacentEmptyVoxelPosition() const {
        return m_targetedAdjacentEmptyVoxel;
    }

    void GameCamera::UpdateTargetedVoxel() {
        // voxel traversal algorithm by https://doi.org/10.2312/egtp.19871000
        glm::vec3 origin = m_camera->GetPosition();
        glm::vec3 dir = glm::normalize(m_camera->GetForward());
        glm::ivec3 voxel = glm::floor(origin);
        glm::ivec3 step = {
            dir.x > 0 ? 1 : -1,
            dir.y > 0 ? 1 : -1,
            dir.z > 0 ? 1 : -1
        };

        glm::vec3 tMax;
        glm::vec3 tDelta;

        for (int i = 0; i < 3; i++) {
            float o = origin[i];
            float d = dir[i];
            int s = step[i];
            float vb = voxel[i] + (s > 0 ? 1.0f : 0.0f);
            tMax[i] = (vb - o) / d;
            tDelta[i] = 1.0f / std::abs(d);
        }

        float maxDist = m_targetedVoxelRange;
        m_cameraInfo.IsTargetingVoxel = false;
        m_targetedAdjacentEmptyVoxel.reset();
        glm::ivec3 lastStepMoved = glm::ivec3(0);

        while (true) {
            glm::vec3 voxelCenter = glm::vec3(voxel) + 0.5f;
            if (glm::distance(origin, voxelCenter) > maxDist) return;

            if (m_world.IsVoxelAt(voxel)) {
                m_cameraInfo.TargetedVoxelX = voxel.x;
                m_cameraInfo.TargetedVoxelY = voxel.y;
                m_cameraInfo.TargetedVoxelZ = voxel.z;
                m_cameraInfo.IsTargetingVoxel = true;
                if (!m_world.IsVoxelAt(voxel - lastStepMoved)) {
                    m_targetedAdjacentEmptyVoxel = voxel - lastStepMoved;
                }
                return;
            }

            if (tMax.x < tMax.y) {
                if (tMax.x < tMax.z) {
                    voxel.x += step.x;
                    tMax.x += tDelta.x;
                    lastStepMoved = glm::ivec3(step.x, 0, 0);
                } else {
                    voxel.z += step.z;
                    tMax.z += tDelta.z;
                    lastStepMoved = glm::ivec3(0, 0, step.z);
                }
            } else {
                if (tMax.y < tMax.z) {
                    voxel.y += step.y;
                    tMax.y += tDelta.y;
                    lastStepMoved = glm::ivec3(0, step.y, 0);
                } else {
                    voxel.z += step.z;
                    tMax.z += tDelta.z;
                    lastStepMoved = glm::ivec3(0, 0, step.z);
                }
            }
        }
    }
} // SpireVoxel
