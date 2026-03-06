#pragma once

#include "pch.h"

namespace Spire {
    class MathsUtils {
    public:
        MathsUtils() = delete;

    public:
        // https://cplusplus.com/forum/general/113069/
        static float Remap(float value, float currentMin, float currentMax, float newMin, float newMax) {
            return newMin + (newMax - newMin) * ((value - currentMin) / (currentMax - currentMin));
        }

        static glm::vec2 Remap(glm::vec2 value, glm::vec2 currentMin, glm::vec2 currentMax, glm::vec2 newMin, glm::vec2 newMax) {
            return glm::vec2(
                Remap(value.x, currentMin.x, currentMax.x, newMin.x, newMax.x),
                Remap(value.y, currentMin.y, currentMax.y, newMin.y, newMax.y)
            );
        }

        template<typename T>
        static float DistanceSquared(const T &a, const T &b) {
            // https://community.khronos.org/t/glm-vector-distancesquare/65282
            T temp = a - b;
            return dot(temp, temp);
        }
    };

    template<>
    inline float MathsUtils::DistanceSquared<float>(const float &a, const float &b) {
        float distance = a - b;
        return distance * distance;
    }
} // Spire
