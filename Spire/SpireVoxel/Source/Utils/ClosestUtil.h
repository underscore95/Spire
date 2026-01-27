#pragma once

#include "EngineIncludes.h"

namespace SpireVoxel {
    class ClosestUtil {
    public:
        ClosestUtil() = delete;

        // Container should be a collection of glm::uvec3
        // Return the closest std::min(maxCount, coords.length()) coords to origin
        template<typename Container>
        [[nodiscard]] static std::vector<glm::uvec3> GetClosestCoords(
            const Container &coords,
            glm::vec3 origin,
            std::size_t maxCount
        ) {
            struct Entry {
                glm::uvec3 coords;
                float distanceSquared;

                bool operator<(const Entry &other) const {
                    return distanceSquared < other.distanceSquared;
                }
            };

            std::priority_queue<Entry> queue;

            for (const glm::uvec3 &c : coords) {
                glm::vec3 delta = static_cast<glm::vec3>(c) - origin;

                float distanceSquared = glm::dot(delta, delta);

                if (queue.size() < maxCount) {
                    queue.push({c, distanceSquared});
                } else if (queue.top().distanceSquared > distanceSquared) {
                    queue.pop();
                    queue.push({c, distanceSquared});
                }
            }

            std::vector<glm::uvec3> result;
            result.reserve(queue.size());

            while (!queue.empty()) {
                result.push_back(queue.top().coords);
                queue.pop();
            }

            return result;
        }
    };
} // SpireVoxel
