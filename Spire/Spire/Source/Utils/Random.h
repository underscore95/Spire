#pragma once

#include "pch.h"

namespace Spire {
    class Random {
    public:
        Random();

    public:
        [[nodiscard]] float RandomFloat() { return RandomFloat(0, 1); }
        [[nodiscard]] float RandomFloat(float min, float max); // min inclusive, max exclusive

        [[nodiscard]] int RandomInt(int min, int max); // min inclusive, max exclusive

        [[nodiscard]] bool RandomBool();

    private:
        std::random_device m_device;
        std::mt19937 m_gen;
    };
} // Spire
