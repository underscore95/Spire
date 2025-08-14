#include "DescriptorManager.h"
#include "DescriptorPool.h"
#include "DescriptorSetsUpdater.h"
#include "Engine/Rendering/RenderingManager.h"

DescriptorManager::DescriptorManager(
    RenderingManager& renderingManager,
    const DescriptorSetLayoutList& layouts
) : m_renderingManager(renderingManager),
    m_layouts(layouts)
{
    m_pool = std::make_unique<DescriptorPool>(m_renderingManager);

    m_descriptorSets = m_pool->Allocate(layouts);
    m_rawLayouts = DescriptorSet::ToLayoutVector(m_descriptorSets);

    m_updater = std::make_unique<DescriptorSetsUpdater>(
        renderingManager.GetDevice(),
        static_cast<glm::u32>(m_descriptorSets.size()),
        m_descriptorSets.data()
    );
    m_updater->Update();
}

DescriptorManager::~DescriptorManager()
{
    m_updater.reset();

    m_pool->Free(m_descriptorSets);
    m_pool.reset();
}

void DescriptorManager::CmdBind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, glm::u32 setIndex,
                                glm::u32 firstSet) const
{
    m_descriptorSets[setIndex].CmdBind(commandBuffer, pipelineLayout, firstSet);
}

const std::vector<VkDescriptorSetLayout>& DescriptorManager::GetRawLayouts() const
{
    return m_rawLayouts;
}