#include "Timer.h"
#include "pch.h"

namespace Spire
{
    void Timer::Start() {
        m_started = true;
        m_startTime = std::chrono::high_resolution_clock::now();
    }

    float Timer::SecondsSinceStart() const {
        assert(m_started);
        const auto duration = std::chrono::high_resolution_clock::now() - m_startTime;
        return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count() / 1000000000.0f;
    }
}