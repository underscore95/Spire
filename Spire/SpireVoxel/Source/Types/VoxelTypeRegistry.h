#pragma once

#include "EngineIncludes.h"
#include "RegisteredVoxelType.h"

namespace Spire {
    class RenderingManager;
}

namespace SpireVoxel {
    class VoxelTypeRegistry {
    public:
        explicit VoxelTypeRegistry(Spire::RenderingManager &renderingManager);

        ~VoxelTypeRegistry();

    public:
        void RegisterType(VoxelTypeInfo type);

        void RegisterTypes(const std::vector<VoxelTypeInfo> &types);

        std::vector<RegisteredVoxelType>::iterator begin();

        std::vector<RegisteredVoxelType>::iterator end();

        DelegateSubscribers<std::span<RegisteredVoxelType> > &GetOnTypesRemovedDelegate();

        DelegateSubscribers<> &GetOnTypesChangedDelegate();

        [[nodiscard]] std::span<RegisteredVoxelType> GetTypes();

        [[nodiscard]] Spire::Descriptor GetVoxelTypesBufferDescriptor(glm::u32 binding);

    private:
        void RecreateVoxelTypesBuffer();

        void DestroyVoxelTypesBuffer();

    private:
        Spire::RenderingManager &m_renderingManager;
        std::vector<RegisteredVoxelType> m_voxelTypes;
        Delegate<std::span<RegisteredVoxelType> > m_onTypesRemovedDelegate; // removed types
        Delegate<> m_onTypesChangedDelegate;
        Spire::VulkanBuffer m_voxelTypesBuffer;
    };
} // SpireVoxel
