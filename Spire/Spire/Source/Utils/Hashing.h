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

    template<typename T, typename... Rest>
    std::size_t Hash(const T& v, Rest... rest) {
        // i randomly generated this seed
        // fun fact its a 0.05% chance to only have one letter
        std::size_t seed = 0x7b63736001704543;
        HashCombine(seed, v, rest...);
        return seed;
    }
} // Spire

// Makes a struct hashable by combining the hashes of its members
// Must be used in the global namespace
// You must pass in the type name (+ any namespaces etc) and then all the fields each prefixed with t.
// for example: MAKE_HASHABLE(glm::ivec3, t.x, t.y, t.z);
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
