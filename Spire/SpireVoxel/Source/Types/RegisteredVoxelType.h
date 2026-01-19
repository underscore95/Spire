#pragma once
#include "VoxelType.h"

namespace SpireVoxel {
    class VoxelTypeRegistry;

    class RegisteredVoxelType {
        friend class VoxelTypeRegistry;

    public:
        explicit RegisteredVoxelType(const VoxelTypeInfo &type) : m_type(type) {
        }

    public:
        std::vector<Spire::VulkanImage> Images;
        glm::u32 FirstImageIndex = UINT32_MAX;

        [[nodiscard]] const VoxelTypeInfo &GetType() const { return m_type; }

    private:
        VoxelTypeInfo m_type;
    };
} // SpireVoxel
