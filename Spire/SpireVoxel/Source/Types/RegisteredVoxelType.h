#pragma once
#include "VoxelType.h"

namespace SpireVoxel {
    class VoxelTypeRegistry;

    class RegisteredVoxelType {
        friend class VoxelTypeRegistry;

    public:
        explicit RegisteredVoxelType(const VoxelType &type) : m_type(type) {
        }

    public:
        Spire::VulkanImage Image;
        glm::u32 FirstImageIndex = UINT32_MAX;

        [[nodiscard]] const VoxelType &GetType() const { return m_type; }

    private:
        VoxelType m_type;
    };
} // SpireVoxel
