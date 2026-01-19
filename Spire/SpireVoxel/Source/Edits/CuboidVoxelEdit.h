#pragma once
#include "IVoxelEdit.h"

namespace SpireVoxel {
    class CuboidVoxelEdit : public IVoxelEdit {
    public:
        struct Edit {
            glm::ivec3 ChunkPosition;
            glm::uvec3 RectOrigin;
            glm::uvec3 RectSize;
        };

        CuboidVoxelEdit(glm::ivec3 origin, glm::uvec3 size, VoxelType voxelType);

    public:
        void Apply(VoxelWorld &world) override;

        static std::unordered_set<glm::ivec3> CalculateAffectedChunkMeshes(const std::vector<Edit> &edits);

        static std::vector<Edit> GenerateEdits(glm::ivec3 origin, glm::uvec3 size);

    private:
        VoxelType m_voxelType;
        std::vector<Edit> m_edits;
        std::unordered_set<glm::ivec3> m_affectedChunkMeshes;
    };
} // SpireVoxel
