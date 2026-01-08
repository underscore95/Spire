#pragma once
#include "pch.h"

namespace Spire {
    inline std::mutex logMutex;

    template<typename... Args>
    void info(const std::string &fmt, Args &&... args) {
        std::unique_lock lock(logMutex);
        std::cout << "[INFO] " << std::vformat(fmt, std::make_format_args(args...)) << std::endl;
    }

    template<typename... Args>
    void warn(const std::string &fmt, Args &&... args) {
        std::unique_lock lock(logMutex);
        std::cout << "[WARN] " << std::vformat(fmt, std::make_format_args(args...)) << std::endl;
    }

    template<typename... Args>
    void error(const std::string &fmt, Args &&... args) {
        std::unique_lock lock(logMutex);
        std::cerr << "[ERROR] " << std::vformat(fmt, std::make_format_args(args...)) << std::endl;
    }
}
