#pragma once

#include "EngineIncludes.h"

namespace SpireVoxel {
    struct DetailLevel {
        // 1 = max detail
        // 2 = chunk is double size
        // 3 = chunk is triple size
        glm::u32 Scale = 1;
    };
} // SpireVoxel
