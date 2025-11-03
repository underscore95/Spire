#include "Engine/Rendering/Scene/SceneImages.h"

#include <format>
#include <vulkan/vulkan_core.h>
#include "Engine/Rendering/RenderingManager.h"
#include "Engine/Rendering/Descriptors/Descriptor.h"
#include "Engine/Rendering/Memory/ImageManager.h"
#include "Engine/Rendering/Memory/VulkanImage.h"

namespace Spire
{
    SceneImages::SceneImages(RenderingManager& renderingManager,
                                 const std::vector<std::string>& imageFileNames)
        : m_renderingManager(renderingManager)
    {
        for (const auto& imageFileName : imageFileNames)
        {
            std::string imagePath = std::format("{}/{}", ASSETS_DIRECTORY, imageFileName);
            m_loadedImages.push_back(m_renderingManager.GetImageManager().CreateImageFromFile(imagePath.c_str()));
        }
    }

    SceneImages::~SceneImages()
    {
        for (auto& image : m_loadedImages)
        {
            m_renderingManager.GetImageManager().DestroyImage(image);
        }
    }

    Descriptor SceneImages::GetDescriptor(glm::u32 binding) const
    {
        return {
            .ResourceType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .Binding = binding,
            .Stages = VK_SHADER_STAGE_FRAGMENT_BIT,
            .NumResources = static_cast<glm::u32>(m_loadedImages.size()),
            .ResourcePtrs = m_loadedImages.data(),
        };
    }

    glm::u32 SceneImages::NumLoadedImages() const
    {
        return m_loadedImages.size();
    }
}