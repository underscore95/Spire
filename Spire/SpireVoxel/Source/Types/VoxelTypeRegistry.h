#pragma once

#include "EngineIncludes.h"
#include "RegisteredVoxelType.h"

namespace Spire {
    class RenderingManager;
}

namespace SpireVoxel {
    // Keeps track of all registered voxel types, e.g. dirt, grass
    class VoxelTypeRegistry {
    public:
        explicit VoxelTypeRegistry(Spire::RenderingManager &renderingManager);

        ~VoxelTypeRegistry();

    public:
        // Register new voxel type
        void RegisterType(VoxelTypeInfo type);

        void RegisterTypes(const std::vector<VoxelTypeInfo> &types);

        // Iterate over all registered types
        std::vector<RegisteredVoxelType>::iterator begin();

        std::vector<RegisteredVoxelType>::iterator end();

        // Delegates
        DelegateSubscribers<std::span<RegisteredVoxelType> > &GetOnTypesRemovedDelegate();

        DelegateSubscribers<> &GetOnTypesChangedDelegate();

        // Get all registered types
        [[nodiscard]] std::span<RegisteredVoxelType> GetTypes();

        // Create descriptor for the type buffer
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
