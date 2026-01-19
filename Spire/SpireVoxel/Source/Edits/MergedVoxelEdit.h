#pragma once
#include "IVoxelEdit.h"

namespace SpireVoxel {
    template<typename T>
    concept VoxelEditType = std::is_base_of_v<IVoxelEdit, T>;

    // Combines multiple IVoxelEdit into a single edit
    class MergedVoxelEdit final : public IVoxelEdit {
    public:
        explicit MergedVoxelEdit(std::vector<std::unique_ptr<IVoxelEdit> > &&edits);

        explicit MergedVoxelEdit() = default;

        template<typename VoxelEditType>
        explicit MergedVoxelEdit(const VoxelEditType &edit) { With(edit); }

    public:
        void Apply(VoxelWorld &world) override;

        MergedVoxelEdit &With(std::unique_ptr<IVoxelEdit> edit);

        template<typename VoxelEditType>
        MergedVoxelEdit &With(const VoxelEditType &edit) {
            m_edits.emplace_back(new VoxelEditType(edit));
            return *this;
        }

    private:
        std::vector<std::unique_ptr<IVoxelEdit>> m_edits;
    };
} // SpireVoxel
