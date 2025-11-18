#include "VoxelImageManager.h"
#include "RegisteredVoxelType.h"
#include "VoxelTypeRegistry.h"
#include "Rendering/Descriptors/Descriptor.h"

namespace SpireVoxel {
    VoxelImageManager::VoxelImageManager(Spire::RenderingManager &renderingManager,
                                         VoxelTypeRegistry &types,
                                         glm::u32 descriptorBinding)
        : m_renderingManager(renderingManager),
          m_types(types),
          m_voxelTypesRemovedSubscriber(m_types.GetOnTypesRemovedDelegate(), [this](std::span<RegisteredVoxelType> removed) { DestroyImagesForTypes(removed); }),
          m_voxelTypesChangedSubscriber(m_types.GetOnTypesChangedDelegate(), [this] { ReloadImages(); }),
          m_descriptorBinding(descriptorBinding) {
    }

    VoxelImageManager::~VoxelImageManager() {
        DestroyImagesForTypes(m_types.GetTypes());
    }

    Spire::Descriptor VoxelImageManager::GetDescriptor() {
        if (!m_descriptor) ReloadImages();
        return *m_descriptor;
    }

    void VoxelImageManager::ReloadImages() {
        m_voxelTypeImages.clear();
        for (RegisteredVoxelType &type : m_types) {
            LoadImageIfNotLoaded(type);
            m_voxelTypeImages.push_back({.Image = &type.Image});
        }

        m_descriptor = Spire::Descriptor{
            .ResourceType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .Binding = m_descriptorBinding,
            .Stages = VK_SHADER_STAGE_FRAGMENT_BIT,
            .Resources = m_voxelTypeImages,
#ifndef NDEBUG
            .DebugName = std::format("Voxel Type Images"),
#endif
        };
    }

    void VoxelImageManager::DestroyImagesForTypes(std::span<RegisteredVoxelType> types) {
        for (RegisteredVoxelType &type : types) {
            m_renderingManager.GetImageManager().DestroyImage(type.Image);
            type.Image = {};
        }
        m_descriptor.reset();
    }

    void VoxelImageManager::LoadImageIfNotLoaded(RegisteredVoxelType &type) const {
        if (type.Image.Image != VK_NULL_HANDLE) return;

        type.Image = m_renderingManager.GetImageManager().CreateImageFromFile(type.GetType().Texture.c_str());
    }
} // SpireVoxel
