#include "ThreadPool.h"

namespace Spire {
    BS::thread_pool<>& ThreadPool::Instance() {
        static BS::thread_pool<> pool;
        return pool;
    }
} // Spire