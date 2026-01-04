#pragma once

#include "ChunkMesher.h"

namespace SpireVoxel {
    class GPUChunkMesher : public ChunkMesher {
    public:
        explicit GPUChunkMesher(Spire::RenderingManager &renderingManager);

        ~GPUChunkMesher() override;

    public:
        [[nodiscard]] std::future<std::vector<VertexData> > Mesh(const std::shared_ptr<Chunk>& chunk) override;

        [[nodiscard]] float GetLoad() override {
            return 1; /*todo*/
        }

    private:
        Spire::RenderingManager &m_renderingManager;
    };
} // SpireVoxel
