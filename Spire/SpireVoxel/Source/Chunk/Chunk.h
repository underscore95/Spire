#pragma once

#include "EngineIncludes.h"
#include "../../Assets/Shaders/ShaderInfo.h"

namespace SpireVoxel {
    class Chunk {
    public:
        void SetVoxel(glm::vec3 pos,std:: int32_t type);
        void SetVoxelRect(glm::vec3 pos, glm::vec3 rectDimensions, std:: int32_t type);

        Spire::Model GenerateModel();

    private:
        std::array<std::int32_t, SPIRE_VOXEL_CHUNK_VOLUME> m_voxelData{};
    };
} // SpireVoxel
