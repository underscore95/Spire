#pragma once
#include "ISamplingOffsets.h"

namespace SpireVoxel {
    // LODSampling python script generates random floats and stores to LODSamplingOffsets.bin
    class SamplingOffsets : public ISamplingOffsets {
    public:
        explicit SamplingOffsets(const std::string &path);

    public:
        [[nodiscard]] float GetOffset(glm::u32 x, glm::u32 y, glm::u32 z) override;

    private:
        std::vector<float> m_offsets;
    };
} // SpireVoxel
