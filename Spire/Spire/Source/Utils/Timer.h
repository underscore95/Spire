#pragma once
#include "pch.h"

namespace Spire
{
    class Timer {
    public:
        Timer(); // Starts automatically

    public:
        void Restart();

        float SecondsSinceStart() const;
        float MillisSinceStart() const;

    private:
        std::chrono::high_resolution_clock::time_point m_startTime;
    };
}