#pragma once

#include "EngineIncludes.h"

namespace SpireVoxel {
    class ISamplingOffsets {
    public:
        virtual ~ISamplingOffsets() = default;

        virtual [[nodiscard]] float GetOffset(glm::u32 x, glm::u32 y, glm::u32 z) = 0; // 0 to 1 range (1 exclusive)
    };
} // SpireVoxel
