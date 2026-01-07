#pragma once

namespace SpireVoxel {
    // What changes to the renderer are required after a chunk mesh regeneration?
    struct WorldEditRequiredChanges {
        // If true, command buffers need to be recreated
        bool RecreateCommandBuffers;

        // |= on every flag
        WorldEditRequiredChanges &operator|=(const WorldEditRequiredChanges &other) {
            RecreateCommandBuffers |= other.RecreateCommandBuffers;
            return *this;
        }

        // Return true if any flag is true
        [[nodiscard]] bool IsAnyChanges() const {
            return RecreateCommandBuffers;
        }
    };
} // SpireVoxel
