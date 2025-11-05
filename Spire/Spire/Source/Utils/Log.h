#pragma once
#include "pch.h"

namespace Spire {
    template<typename... Args>
    void info(const std::string &fmt, Args &&... args) {
        std::cout << "[INFO] " << std::vformat(fmt, std::make_format_args(args...)) << '\n';
    }

    template<typename... Args>
    void warn(const std::string &fmt, Args &&... args) {
        std::cout << "[WARN] " << std::vformat(fmt, std::make_format_args(args...)) << '\n';
    }

    template<typename... Args>
    void error(const std::string &fmt, Args &&... args) {
        std::cerr << "[ERROR] " << std::vformat(fmt, std::make_format_args(args...)) << '\n';
    }
}
