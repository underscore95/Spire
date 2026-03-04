#pragma once
#include <iostream>
#include <mutex>
#include <string>

inline void log(std::string s) {
    static std::mutex m;
    std::unique_lock l(m);
    std::cout << s << "\n";
}
