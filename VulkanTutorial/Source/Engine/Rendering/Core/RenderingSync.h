#pragma once

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>

namespace Spire
{
    class RenderingManager;

    class RenderingSync
    {
    public:
        explicit RenderingSync(RenderingManager& renderingManager);
        ~RenderingSync();

    public:
        [[nodiscard]] VkSemaphore CreateSemaphore();

        void DestroySemaphore(VkSemaphore semaphore);

        void ImageMemoryBarrier(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout,
                                VkImageLayout newLayout) const;

    private:
        RenderingManager& m_renderingManager;
        glm::u32 m_numAllocatedSemaphores = 0;
    };
}
