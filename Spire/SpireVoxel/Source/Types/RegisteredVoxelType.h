#pragma once
#include "VoxelTypeInfo.h"

namespace SpireVoxel {
    class VoxelTypeRegistry;

    // Represents a voxel type that has been registered
    // See VoxelTypeInfo and GPUVoxelType
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
