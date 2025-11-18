#pragma once

#include "EngineIncludes.h"
#include "RegisteredVoxelType.h"

namespace Spire {
    struct VulkanImage;
    class RenderingManager;
}

namespace SpireVoxel {
    class VoxelTypeRegistry;

    class VoxelImageManager {
    public:
        explicit VoxelImageManager(Spire::RenderingManager &renderingManager,
                                   VoxelTypeRegistry &types,
                                   glm::u32 descriptorBinding);

        ~VoxelImageManager();

    public:
        [[nodiscard]] Spire::Descriptor GetDescriptor();

    private:
        void ReloadImages();

        void DestroyImagesForTypes(std::span<RegisteredVoxelType> types);

        void LoadImagesIfNotLoaded(RegisteredVoxelType &type) const;

    private:
        Spire::RenderingManager &m_renderingManager;
        VoxelTypeRegistry &m_types;
        std::optional<Spire::Descriptor> m_descriptor;
        LocalDelegateSubscriber<std::span<RegisteredVoxelType>> m_voxelTypesRemovedSubscriber;
        LocalDelegateSubscriber<> m_voxelTypesChangedSubscriber;
        glm::u32 m_descriptorBinding;
        std::vector<Spire::Descriptor::ResourcePtr> m_voxelTypeImages;
    };
} // SpireVoxel
