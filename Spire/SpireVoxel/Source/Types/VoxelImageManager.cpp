#include "VoxelImageManager.h"
#include "RegisteredVoxelType.h"
#include "VoxelTypeRegistry.h"
#include "../../Assets/Shaders/ShaderInfo.h"
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
            LoadImagesIfNotLoaded(type);
            type.FirstImageIndex = m_voxelTypeImages.size();
            for (const auto& image : type.Images) {
                m_voxelTypeImages.push_back({.Image = &image});
            }
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
            for (auto& image : type.Images) {
                if (image.Image != VK_NULL_HANDLE) {
                    m_renderingManager.GetImageManager().DestroyImage(image);
                    image = {};
                }
            }
            type.FirstImageIndex = UINT32_MAX;
        }
        m_descriptor.reset();
    }

    void VoxelImageManager::LoadImagesIfNotLoaded(RegisteredVoxelType &type) const {
        auto &images = type.Images;

        // check no images are loaded
        bool isAnyImageLoaded = false;
        for (const auto &image : images) {
            if (image.Image == VK_NULL_HANDLE) {
                isAnyImageLoaded = true;
                break;
            }
        }
        if (isAnyImageLoaded) {
            // check that some aren't partial loaded
            for (const auto &image : images) {
                assert(image.Image != VK_NULL_HANDLE);
            }
            return;
        }

        images.clear();

        // load images
        assert(type.GetType().Textures.size() == GetNumImages(type.GetType().VoxelFaceLayout));
        for (const std::string &path : type.GetType().Textures) {
            images.push_back(m_renderingManager.GetImageManager().CreateImageFromFile(path.c_str()));
        }
    }
} // SpireVoxel
