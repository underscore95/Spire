#include "MergedVoxelEdit.h"

namespace SpireVoxel {
    MergedVoxelEdit::MergedVoxelEdit(std::vector<std::unique_ptr<IVoxelEdit> > &&edits)
        : m_edits(std::move(edits)) {
    }

    void MergedVoxelEdit::Apply(VoxelWorld &world) {
        if (m_edits.empty()) Spire::warn("Applying MergedVoxelEdit with no edits");
        for (auto &edit : m_edits) {
            edit->Apply(world);
        }
    }

    MergedVoxelEdit &MergedVoxelEdit::With(std::unique_ptr<IVoxelEdit> edit) {
        m_edits.push_back(std::move(edit));
        return *this;
    }
} // SpireVoxel
