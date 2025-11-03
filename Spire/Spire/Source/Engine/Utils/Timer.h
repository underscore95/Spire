#pragma once
#include <chrono>

namespace Spire
{
    class Timer {
    public:
        Timer() = default;

    public:
        void Start();

        float SecondsSinceStart() const;

    private:
        std::chrono::high_resolution_clock::time_point m_startTime;
        bool m_started = false;
    };
}