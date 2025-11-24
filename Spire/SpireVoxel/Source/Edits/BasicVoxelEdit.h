#pragma once

#include "IVoxelEdit.h"

namespace SpireVoxel {
    class BasicVoxelEdit : public IVoxelEdit {
    public:
        struct Edit {
            glm::ivec3 Position;
            glm::u32 Type;
        };

    public:
        explicit BasicVoxelEdit(const std::vector<Edit> &edits);

    public:
        void Apply(VoxelWorld &world) override;

    private:
        std::vector<Edit> m_edits;
    };
} // SpireVoxel
