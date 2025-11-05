#pragma once

#include "pch.h"

namespace Spire {
    class Version {
    public:
        Version(unsigned int major, unsigned int minor, unsigned int patch);

    public:
        // Parse string in format major.minor.patch, optionally with a v at beginning
        [[nodiscard]] static std::optional<Version> FromString(const std::string &str);

        bool operator==(const Version & version) const;

    private:
        [[nodiscard]] static bool IsDigit(char c);
        [[nodiscard]] static int ToDigit(char c);

    private:
        unsigned int m_major;
        unsigned int m_minor;
        unsigned int m_patch;
    };
}
