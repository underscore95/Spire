#pragma once
#include "../../Assets/Shaders/ShaderInfo.h"
#include "Rendering/Core/RenderingCommandManager.h"

namespace SpireVoxel {
    struct ChunkDrawParams {
        static constexpr glm::u32 COMMANDS_PER_CHUNK = SPIRE_VOXEL_NUM_FACES;
        static constexpr glm::u32 STRIDE = sizeof(VkDrawIndirectCommand);

        // https://sakibsaikia.github.io/graphics/2017/08/18/Going-Indirect-On-UE3.html
        // Some faces will have 0 instance count
        // These have very small overhead
        std::array<VkDrawIndirectCommand, COMMANDS_PER_CHUNK> Commands;
    };
} // SpireVoxel
