#pragma once

#include "ChunkMesher.h"

namespace SpireVoxel {
    class GPUChunkMesher : public ChunkMesher {

    public:
        explicit GPUChunkMesher(Spire::RenderingManager &renderingManager);

        ~GPUChunkMesher() override;

    protected:
        [[nodiscard]] std::vector<VertexData> GenerateMesh(const std::shared_ptr<Chunk> &chunk, glm::u32 threadId) override;

        [[nodiscard]] float GetLoad() override;

        [[nodiscard]] glm::u32 GetMaxParallelisation() const override;

        void OnInit() override;

    private:
        Spire::RenderingManager &m_renderingManager;
      // Each thread uses a single element of the vector (using thread id as index)
std::vector<        VkCommandBuffer> m_commandBuffers;
    };
} // SpireVoxel
