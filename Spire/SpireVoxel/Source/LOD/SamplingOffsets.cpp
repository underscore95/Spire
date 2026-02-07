#include "SamplingOffsets.h"

#include "../../Assets/Shaders/ShaderInfo.h"
#include "Utils/Log.h"

namespace SpireVoxel {
    SamplingOffsets::SamplingOffsets(const std::string &path) {
        Spire::Timer timer;
        m_offsets.resize(SPIRE_VOXEL_CHUNK_VOLUME);
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file) {
            Spire::error("Failed to load sampling offsets: {} (open fail)", path);
            return;
        }
        file.seekg(0, std::ios::beg);
        if (!file.read(reinterpret_cast<char *>(m_offsets.data()), static_cast<glm::u32>(m_offsets.size() * sizeof(m_offsets[0])))) {
            Spire::error("Failed to load sampling offsets: {} (read fail)", path);
        }

        Spire::info("Loaded sampling offsets in {} ms ({} KB)", timer.MillisSinceStart(), m_offsets.size() * sizeof(m_offsets[0]) / 1024);
    }

    float SamplingOffsets::GetOffset(glm::u32 x, glm::u32 y, glm::u32 z) {
        float offset = m_offsets[SPIRE_VOXEL_POSITION_XYZ_TO_INDEX(x, y, z)];
        return offset;
    }
} // SpireVoxel
