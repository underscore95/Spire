#include "CommandBufferVector.h"

#include "Rendering/RenderingManager.h"
#include "Rendering/Core/RenderingCommandManager.h"

namespace Spire {
    CommandBufferVector::CommandBufferVector(RenderingManager &renderingManager,
                                             glm::u32 numCommandBuffers)
        : m_renderingManager(renderingManager) {
        m_commandBuffers.resize(numCommandBuffers);
        m_renderingManager.GetCommandManager().CreateCommandBuffers(numCommandBuffers, m_commandBuffers.data());
    }

    CommandBufferVector::~CommandBufferVector() {
        m_renderingManager.GetCommandManager().FreeCommandBuffers(m_commandBuffers.size(), m_commandBuffers.data());
    }

    std::size_t CommandBufferVector::Size() const {
        return m_commandBuffers.size();
    }

    VkCommandBuffer &CommandBufferVector::operator[](std::size_t i) {
        return m_commandBuffers[i];
    }

    bool CommandBufferVector::Empty() const {
        return Size() == 0;
    }
} // Spire
