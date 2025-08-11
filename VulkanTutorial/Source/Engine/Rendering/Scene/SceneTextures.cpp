#include "Engine/Rendering/Scene/SceneTextures.h"

#include <format>
#include <vulkan/vulkan_core.h>

#include "Engine/Rendering/Core/PipelineDescriptorSetsManager.h"
#include "Engine/Rendering/RenderingManager.h"
#include "Engine/Rendering/Memory/TextureManager.h"
#include "Engine/Rendering/Memory/VulkanImage.h"

SceneTextures::SceneTextures(RenderingManager& renderingManager,
                             const std::vector<std::string>& textureFileNames)
    : m_renderingManager(renderingManager)
{
    for (const auto& textureFileName : textureFileNames)
    {
        std::string texturePath = std::format("{}/{}", ASSETS_DIRECTORY, textureFileName);
        m_loadedTextures.push_back(m_renderingManager.GetTextureManager().CreateImageFromFile(texturePath.c_str()));
    }
}

SceneTextures::~SceneTextures()
{
    for (auto& tex : m_loadedTextures)
    {
        m_renderingManager.GetTextureManager().DestroyImage(tex);
    }
}

PipelineResourceInfo SceneTextures::GetPipelineResourceInfo(glm::u32 binding)
{
    return {
        .ResourceType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .Binding = binding,
        .Stages = VK_SHADER_STAGE_FRAGMENT_BIT,
        .SameResourceForAllFrames = true,
        .ResourcePtrs = m_loadedTextures.data(),
        .NumDescriptors = static_cast<glm::u32>(m_loadedTextures.size())
    };
}

glm::u32 SceneTextures::NumLoadedTextures() const
{
    return m_loadedTextures.size();
}
