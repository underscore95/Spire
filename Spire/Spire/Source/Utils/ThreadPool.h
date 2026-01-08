#pragma once

#include "BS_thread_pool.hpp"
#include "MacroDisableCopy.h"

namespace Spire {
    class ThreadPool {
    public:
        ThreadPool() = delete;

        DISABLE_COPY(ThreadPool);

    public:
        static BS::thread_pool<> &Instance();
    };
} // Spire
