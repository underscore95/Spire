#pragma once

#include "EngineIncludes.h"

// This file was generated automatically by Tooling/AOLookup

namespace SpireVoxel {
    inline void GetAmbientOcclusionOffsetVectors(glm::u32 face, glm::u32 vertexPos, glm::ivec3 &i, glm::ivec3 &j, glm::ivec3 &k) {
        assert(face < 6);
        assert(vertexPos < 4);
        constexpr std::array AO_OFFSET_TABLE = {
            glm::ivec3{1, 0, 0},
            glm::ivec3{0, -1, 0},
            glm::ivec3{0, 0, 1},
            glm::ivec3{1, 0, 0},
            glm::ivec3{0, -1, 0},
            glm::ivec3{0, 0, -1},
            glm::ivec3{1, 0, 0},
            glm::ivec3{0, 1, 0},
            glm::ivec3{0, 0, -1},
            glm::ivec3{1, 0, 0},
            glm::ivec3{0, 1, 0},
            glm::ivec3{0, 0, 1},
            glm::ivec3{-1, 0, 0},
            glm::ivec3{0, -1, 0},
            glm::ivec3{0, 0, -1},
            glm::ivec3{-1, 0, 0},
            glm::ivec3{0, -1, 0},
            glm::ivec3{0, 0, 1},
            glm::ivec3{-1, 0, 0},
            glm::ivec3{0, 1, 0},
            glm::ivec3{0, 0, 1},
            glm::ivec3{-1, 0, 0},
            glm::ivec3{0, 1, 0},
            glm::ivec3{0, 0, -1},
            glm::ivec3{0, 1, 0},
            glm::ivec3{0, 0, 1},
            glm::ivec3{-1, 0, 0},
            glm::ivec3{0, 1, 0},
            glm::ivec3{0, 0, 1},
            glm::ivec3{1, 0, 0},
            glm::ivec3{0, 1, 0},
            glm::ivec3{0, 0, -1},
            glm::ivec3{1, 0, 0},
            glm::ivec3{0, 1, 0},
            glm::ivec3{0, 0, -1},
            glm::ivec3{-1, 0, 0},
            glm::ivec3{0, -1, 0},
            glm::ivec3{0, 0, -1},
            glm::ivec3{-1, 0, 0},
            glm::ivec3{0, -1, 0},
            glm::ivec3{0, 0, -1},
            glm::ivec3{1, 0, 0},
            glm::ivec3{0, -1, 0},
            glm::ivec3{0, 0, 1},
            glm::ivec3{1, 0, 0},
            glm::ivec3{0, -1, 0},
            glm::ivec3{0, 0, 1},
            glm::ivec3{-1, 0, 0},
            glm::ivec3{0, 0, 1},
            glm::ivec3{-1, 0, 0},
            glm::ivec3{0, -1, 0},
            glm::ivec3{0, 0, 1},
            glm::ivec3{1, 0, 0},
            glm::ivec3{0, -1, 0},
            glm::ivec3{0, 0, 1},
            glm::ivec3{1, 0, 0},
            glm::ivec3{0, 1, 0},
            glm::ivec3{0, 0, 1},
            glm::ivec3{-1, 0, 0},
            glm::ivec3{0, 1, 0},
            glm::ivec3{0, 0, -1},
            glm::ivec3{1, 0, 0},
            glm::ivec3{0, -1, 0},
            glm::ivec3{0, 0, -1},
            glm::ivec3{-1, 0, 0},
            glm::ivec3{0, -1, 0},
            glm::ivec3{0, 0, -1},
            glm::ivec3{-1, 0, 0},
            glm::ivec3{0, 1, 0},
            glm::ivec3{0, 0, -1},
            glm::ivec3{1, 0, 0},
            glm::ivec3{0, 1, 0},
        };

        i = AO_OFFSET_TABLE[face * 4 * 3 + vertexPos * 3 + 0];
        j = AO_OFFSET_TABLE[face * 4 * 3 + vertexPos * 3 + 1];
        k = AO_OFFSET_TABLE[face * 4 * 3 + vertexPos * 3 + 2];
    }
}
