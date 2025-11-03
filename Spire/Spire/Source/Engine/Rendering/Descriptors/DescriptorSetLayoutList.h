#pragma once

#include <vector>
#include "Descriptor.h"

namespace Spire
{
    typedef std::vector<Descriptor> DescriptorSetLayout;
    typedef std::vector<PerImageDescriptor> PerImageDescriptorSetLayout;

    class DescriptorSetLayoutList
    {
    public:
        explicit DescriptorSetLayoutList(glm::u32 numSwapchainImages);

        glm::u32 Push(const DescriptorSetLayout& layout);

        glm::u32 Push(const PerImageDescriptorSetLayout& layout);

        const std::vector<DescriptorSetLayout>& GetDescriptorSets() const;

        const DescriptorSetLayout& GetSet(glm::u32 index) const;

        glm::u32 Size() const;

        bool IsSetPerImage(glm::u32 setIndex) const;

    private:
        glm::u32 m_numSwapchainImages;
        std::vector<DescriptorSetLayout> m_descriptorSets;

        struct DescriptorSetAdditionalInfo
        {
            bool IsPerImage;
            bool IsFirstSetIfPerImage;
        };
        std::vector<DescriptorSetAdditionalInfo> m_additionalDescriptorSetInfo;
    };
}