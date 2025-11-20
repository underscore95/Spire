#pragma once

#include "RenderInfo.h"
#include "EngineIncludes.h"
#include "Chunk/VoxelWorld.h"
#include "../Assets/Shaders/ShaderInfo.h"

namespace SpireVoxel {
    class GameCamera {
    public:
        explicit GameCamera(const Spire::Engine &engine, VoxelWorld &world);

        void Update();

        void Render(const RenderInfo &renderInfo);

        Spire::Camera &GetCamera() const;

        Spire::PerImageDescriptor GetDescriptor(glm::u32 binding) const;

        void SetTargetedVoxelRange(glm::u32 range);

        [[nodiscard]] const CameraInfo &GetCameraInfo() const;

        [[nodiscard]] std::optional<glm::ivec3> GetTargetedVoxelPosition() const;
        [[nodiscard]] std::optional<glm::ivec3> GetTargetedAdjacentEmptyVoxelPosition() const;

    private:
        void UpdateTargetedVoxel();

    private:
        const Spire::Engine &m_engine;
        std::unique_ptr<Spire::PerImageBuffer> m_uniformBuffer = nullptr;
        std::unique_ptr<Spire::Camera> m_camera;
        CameraInfo m_cameraInfo;
        VoxelWorld &m_world;
        glm::u32 m_targetedVoxelRange = 10;
        std::optional<glm::ivec3> m_targetedAdjacentEmptyVoxel; // this is the voxel to the F of the targeted voxel where F is the face of the targeted voxel we are looking at, only present if the voxel is empty
    };
} // SpireVoxel
