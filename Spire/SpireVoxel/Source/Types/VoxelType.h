#pragma once

#include "EngineIncludes.h"

namespace SpireVoxel {
    struct VoxelType {
        glm::u32 Id;
        std::vector<std::string> Textures;
        glm::u32 VoxelFaceLayout;
    };
} // SpireVoxel
