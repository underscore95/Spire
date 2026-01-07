#pragma once

namespace SpireVoxel {
    // What changes to the renderer are required after a chunk mesh regeneration?
    struct WorldEditRequiredChanges {
        bool RecreatePipeline;
        bool RecreateOnlyCommandBuffers;

        // |= on every flag
        WorldEditRequiredChanges &operator|=(const WorldEditRequiredChanges &other) {
            RecreatePipeline |= other.RecreatePipeline;
            RecreateOnlyCommandBuffers |= other.RecreateOnlyCommandBuffers;
            return *this;
        }

        // Return true if any flag is true
        [[nodiscard]] bool IsAnyChanges() const {
            return RecreatePipeline | RecreateOnlyCommandBuffers;
        }
    };
} // SpireVoxel
