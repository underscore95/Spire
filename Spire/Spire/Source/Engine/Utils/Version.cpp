#include "Version.h"

#include <array>

namespace Spire {
    Version::Version(unsigned int major,
                     unsigned int minor,
                     unsigned int patch)
        : m_major(major),
          m_minor(minor),
          m_patch(patch) {
    }

    std::optional<Version> Version::FromString(const std::string &str) {
        if (str.empty()) return {};
        std::size_t firstIndex = str[0] == 'v' || str[0] == 'V' ? 1 : 0;

        bool foundMajor = false;
        bool foundMinor = false;
        std::optional<Version> version = std::make_optional<Version>(0, 0, 0);
        for (size_t i = firstIndex; i < str.size(); i++) {
            if (IsDigit(str[i])) {
                if (!foundMajor) version->m_major = version->m_major * 10 + ToDigit(str[i]);
                else if (!foundMinor)version->m_minor = version->m_minor * 10 + ToDigit(str[i]);
                else version->m_patch = version->m_patch * 10 + ToDigit(str[i]);
                continue;
            }

            if (str[i] == '.') {
                if (!foundMajor) foundMajor = true;
                else if (!foundMinor) foundMinor = true;
                else return {}; // . after the patch
                continue;
            }

            return {}; // invalid character
        }

        return version;
    }

    bool Version::operator==(const Version &version) const {
        return version.m_major == m_major && version.m_minor == m_minor && version.m_patch == m_patch;
    }

    bool Version::IsDigit(char c) {
        return c >= '0' && c <= '9';
    }

    int Version::ToDigit(char c) {
        return c - '0';
    }
} // Spire
