#pragma once

#include "EngineIncludes.h"

namespace SpireVoxel {
    // Represents a voxel type to be registered
    // See GPUVoxelType and RegisteredVoxelType for more information
    struct VoxelTypeInfo {
        glm::u32 Id;
        std::vector<std::string> Textures;
        glm::u32 VoxelFaceLayout;
    };
} // SpireVoxel
