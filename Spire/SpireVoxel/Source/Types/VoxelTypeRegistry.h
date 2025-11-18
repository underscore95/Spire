#pragma once

#include "EngineIncludes.h"
#include "RegisteredVoxelType.h"

namespace Spire {
    class RenderingManager;
}

namespace SpireVoxel {
    class VoxelTypeRegistry {
    public:
        VoxelTypeRegistry();

    public:
        void RegisterType(VoxelType type);

        void RegisterTypes(const std::vector<VoxelType> &types);

        std::vector<RegisteredVoxelType>::iterator begin();

        std::vector<RegisteredVoxelType>::iterator end();

        DelegateSubscribers<std::span<RegisteredVoxelType> > &GetOnTypesRemovedDelegate();

        DelegateSubscribers<> &GetOnTypesChangedDelegate();

        [[nodiscard]] std::span<RegisteredVoxelType> GetTypes();

    private:
        std::vector<RegisteredVoxelType> m_voxelTypes;
        Delegate<std::span<RegisteredVoxelType> > m_onTypesRemovedDelegate; // removed types
        Delegate<> m_onTypesChangedDelegate;
    };
} // SpireVoxel
