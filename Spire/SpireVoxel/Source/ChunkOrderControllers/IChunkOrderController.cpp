#include "IChunkOrderController.h"

glm::ivec3 SpireVoxel::IChunkOrderController::GetSpiral(glm::u32 index) {
    // https://learning.oreilly.com/library/view/concrete-mathematics-a/9780134389974/
    // Chapter 3 Exercise 40
    // Copilot generated based on given formulas

    auto n = static_cast<glm::i32>(index);
    glm::i32 N = n + 1;
    if (N == 1) return {0, 0, 0};

    float root = std::sqrt(static_cast<float>(N));
    auto k = static_cast<glm::i32>(std::ceil((root - 1.0f) / 2.0f));
    glm::i32 t = 2 * k + 1;
    glm::i32 tMinus1 = t - 1;
    glm::i32 m = t * t;

    glm::i32 x, z;

    if (N >= m - tMinus1) {
        x = k - (m - N);
        z = -k;
        return {x, 0, z};
    }

    m -= tMinus1;
    if (N >= m - tMinus1) {
        x = -k;
        z = -k + (m - N);
        return {x, 0, z};
    }

    m -= tMinus1;
    if (N >= m - tMinus1) {
        x = -k + (m - N);
        z = k;
        return {x, 0, z};
    }

    m -= tMinus1;
    x = k;
    z = k - (m - N);
    return {x, 0, z};
}
