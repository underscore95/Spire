#pragma once

#include "EngineIncludes.h"
#include "Chunk/VoxelType.h"

namespace SpireVoxel {
    // Represents a voxel type to be registered
    // See GPUVoxelType and RegisteredVoxelType for more information
    struct VoxelTypeInfo {
        VoxelType Id;
        std::vector<std::string> Textures;
        glm::u32 VoxelFaceLayout;
    };
} // SpireVoxel
