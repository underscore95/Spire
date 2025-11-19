#include "Random.h"

namespace Spire {
    Random::Random() : m_gen(m_device()) {
    }

    float Random::RandomFloat(float min, float max) {
        std::uniform_real_distribution<float> dist(min, max);
        return dist(m_gen);
    }

    int Random::RandomInt(int min, int max) {
        std::uniform_int_distribution<int> dist(min, max - 1);
        return dist(m_gen);
    }

    bool Random::RandomBool() {
        return RandomInt(0, 1 + 1) == 0;
    }
} // Spire
