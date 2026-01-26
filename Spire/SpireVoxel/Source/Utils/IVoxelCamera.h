#pragma once

#include "EngineIncludes.h"

namespace SpireVoxel {
    struct CameraInfo;

    struct RenderInfo {
        glm::u32 ImageIndex;
    };

    // Camera for voxel renderer
    // you can use Spire camera or your own
    class IVoxelCamera {
    public:
        virtual ~IVoxelCamera() = default;

        [[nodiscard]] virtual CameraInfo GetCameraInfo() const = 0;

        [[nodiscard]] virtual glm::vec3 GetPosition() const = 0;

        virtual void Render(const RenderInfo &renderInfo) = 0;

        [[nodiscard]] virtual Spire::PerImageDescriptor GetDescriptor(glm::u32 binding) const = 0;
    };
} // SpireVoxel
