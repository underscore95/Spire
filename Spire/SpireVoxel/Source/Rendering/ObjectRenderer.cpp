#include "ObjectRenderer.h"
#include "../../Assets/Shaders/ShaderInfo.h"

namespace SpireVoxel {
    ObjectRenderer::ObjectRenderer(
        Spire::SceneModels &sceneModels,
        Spire::RenderingManager &renderingManager,
        glm::u32 modelIndex,
        glm::u32 modelDataSize,
        glm::u32 maxInstances
    ) : m_renderingManager(renderingManager),
        m_sceneModels(sceneModels),
        m_modelIndex(modelIndex),
        m_modelDataSize(modelDataSize),
        m_maxInstances(maxInstances) {
        m_modelDataBuffer = m_renderingManager.GetBufferManager().CreateStorageBuffers(maxInstances * modelDataSize, maxInstances, nullptr);
    }

    ObjectRenderer::~ObjectRenderer() = default;

    void ObjectRenderer::SetModelData(glm::u32 imageIndex, glm::u32 index, const void *data) {
        SetModelDatas(imageIndex, index, 1, data);
    }

    void ObjectRenderer::SetModelDatas(glm::u32 imageIndex, glm::u32 startIndex, glm::u32 count, const void *data) {
        assert(count > 0);
        assert(startIndex < m_maxInstances);
        m_modelDataBuffer->Update(imageIndex, data, m_modelDataSize * count, startIndex);
    }

    void ObjectRenderer::CmdRender(VkCommandBuffer commandBuffer, const Spire::GraphicsPipeline &pipeline,
                                   glm::u32 instances, bool bindIndexBuffer) const {
        if (bindIndexBuffer) m_sceneModels.CmdBindIndexBuffer(commandBuffer);
        m_sceneModels.CmdRenderModels(commandBuffer, pipeline, m_modelIndex, instances);
    }

    Spire::PerImageDescriptor ObjectRenderer::GetDescriptor() const {
        return m_renderingManager.GetDescriptorCreator().CreatePerImageStorageBuffer(
            SPIRE_SHADER_BINDINGS_MODEL_DATA_SSBO_BINDING,
            *m_modelDataBuffer,
            VK_SHADER_STAGE_VERTEX_BIT
        );
    }
} // SpireVoxel
