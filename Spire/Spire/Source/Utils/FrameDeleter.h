#pragma once

#include "pch.h"

namespace Spire {
    template<typename T>
    class FrameDeleter {
    public:
        // Keeps the variable around until the specified number of frames has elapsed
        void Push(T &&t, glm::u32 numFrames) {
            m_toDelete.emplace_back(std::move(t), numFrames);
        }

        void Update() {
            for (std::size_t i = 0; i < m_toDelete.size(); i++) {
                if (m_toDelete[i].second == 0) {
                    // delete
                    m_toDelete[i] = std::move(m_toDelete.back());
                    m_toDelete.pop_back();
                } else {
                    m_toDelete[i].second--;
                }
            }
        }

    private:
        std::vector<std::pair<T, glm::u32> > m_toDelete;
    };
} // Spire
