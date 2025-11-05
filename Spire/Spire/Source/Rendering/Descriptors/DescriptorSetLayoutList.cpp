#include "DescriptorSetLayoutList.h"

namespace Spire
{
    DescriptorSetLayoutList::DescriptorSetLayoutList(glm::u32 numSwapchainImages)
        : m_numSwapchainImages(numSwapchainImages)
    {
    }

    glm::u32 DescriptorSetLayoutList::Push(const DescriptorSetLayout& layout)
    {
        m_descriptorSets.push_back(layout);
        m_additionalDescriptorSetInfo.push_back({
            .IsPerImage = false,
            .IsFirstSetIfPerImage = false
        });
        return m_descriptorSets.size() - 1;
    }

    glm::u32 DescriptorSetLayoutList::Push(const PerImageDescriptorSetLayout& layout)
    {
        for (glm::u32 imageIndex = 0; imageIndex < m_numSwapchainImages; imageIndex++)
        {
            DescriptorSetLayout imageLayout;
            imageLayout.reserve(layout.size());
            for (const PerImageDescriptor& perImageDescriptor : layout)
            {
                imageLayout.push_back(perImageDescriptor.Descriptors[imageIndex]);
            }

            m_descriptorSets.push_back(imageLayout);
            m_additionalDescriptorSetInfo.push_back({
                .IsPerImage = true,
                .IsFirstSetIfPerImage = imageIndex == 0
            });
        }
        return m_descriptorSets.size() - m_numSwapchainImages;
    }

    const std::vector<DescriptorSetLayout>& DescriptorSetLayoutList::GetDescriptorSets() const
    {
        return m_descriptorSets;
    }

    const DescriptorSetLayout& DescriptorSetLayoutList::GetSet(glm::u32 index) const
    {
        assert(index < m_descriptorSets.size());
        return m_descriptorSets[index];
    }

    glm::u32 DescriptorSetLayoutList::Size() const
    {
        return m_descriptorSets.size();
    }

    bool DescriptorSetLayoutList::IsSetPerImage(glm::u32 setIndex) const
    {
        assert(setIndex < m_additionalDescriptorSetInfo.size());
        const DescriptorSetAdditionalInfo& info = m_additionalDescriptorSetInfo[setIndex];
        if (info.IsPerImage)
            assert(info.IsFirstSetIfPerImage);
        return info.IsPerImage;
    }
}