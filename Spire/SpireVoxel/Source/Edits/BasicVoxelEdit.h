#pragma once

#include "IVoxelEdit.h"

namespace SpireVoxel {

    // Basic edit that changes 1 or more voxels to 1 or more voxel types
    class BasicVoxelEdit : public IVoxelEdit {
    public:
        struct Edit {
            glm::ivec3 Position;
            VoxelType Type;
        };

    public:
        explicit BasicVoxelEdit(const std::vector<Edit> &edits);

        explicit BasicVoxelEdit(const Edit &edit);

        explicit BasicVoxelEdit(glm::ivec3 position, VoxelType type);

    public:
        void Apply(VoxelWorld &world) override;

    private:
        std::vector<Edit> m_edits;
    };
} // SpireVoxel
