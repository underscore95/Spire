#pragma once

#include "pch.h"

// From: https://github.com/underscore95/Chimp/blob/main/Chimp/ChimpFramework/Source/api/utils/HashCombine.h
namespace Spire {
    // https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x

    inline void HashCombine(std::size_t &seed) {
    }

    template<typename T, typename... Rest>
    void HashCombine(std::size_t &seed, const T &v, Rest... rest) {
        std::hash<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        HashCombine(seed, rest...);
    }
} // Spire

// Makes a struct hashable by combining the hashes of its members
// Must be used in the global namespace
#define MAKE_HASHABLE(type, ...) \
    template<> struct std::hash<type> {\
        std::size_t operator()(const type &t) const {\
            std::size_t ret = 0;\
            Spire::HashCombine(ret, __VA_ARGS__);\
            return ret;\
        }\
    };

// Hash common types
MAKE_HASHABLE(glm::ivec3, t.x, t.y, t.z);
MAKE_HASHABLE(glm::uvec3, t.x, t.y, t.z);
