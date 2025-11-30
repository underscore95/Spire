#include "VoxelTypeRegistry.h"

#include "RegisteredVoxelType.h"
#include "VoxelType.h"
#include "Chunk/Chunk.h"

namespace SpireVoxel {
    VoxelTypeRegistry::VoxelTypeRegistry(Spire::RenderingManager &renderingManager)
        : m_renderingManager(renderingManager),
          m_onTypesRemovedDelegate(),
          m_onTypesChangedDelegate() {
    }

    VoxelTypeRegistry::~VoxelTypeRegistry() {
        DestroyVoxelTypesBuffer();
    }

    void VoxelTypeRegistry::RegisterType(VoxelType type) {
        RegisterTypes({type});
    }

    std::optional<RegisteredVoxelType> RemoveVoxelTypeFromVectorMayReorder(glm::u32 id, std::vector<RegisteredVoxelType> &types) {
        // won't work if the vector contains multiple of the id
        for (size_t i = 0; i < types.size(); i++) {
            if (types[i].GetType().Id == id) {
                RegisteredVoxelType found = types[i];
                types[i] = types.back();
                types.pop_back();
                return found;
            }
        }
        return {};
    }

    void VoxelTypeRegistry::RegisterTypes(const std::vector<VoxelType> &types) {
        assert(m_voxelTypes.empty()); // may work but modifying a second time is untested

        std::vector<RegisteredVoxelType> removed;

        for (const VoxelType &type : types) {
            assert(type.Id != VOXEL_TYPE_AIR);

            std::optional<RegisteredVoxelType> oldType = RemoveVoxelTypeFromVectorMayReorder(type.Id, m_voxelTypes);
            if (oldType) {
                removed.push_back(*oldType);
            }

            m_voxelTypes.emplace_back(type);
        }

        m_onTypesRemovedDelegate.Broadcast(std::span{removed});
        m_onTypesChangedDelegate.Broadcast();

        RecreateVoxelTypesBuffer();
    }

    std::vector<RegisteredVoxelType>::iterator VoxelTypeRegistry::begin() {
        return m_voxelTypes.begin();
    }

    std::vector<RegisteredVoxelType>::iterator VoxelTypeRegistry::end() {
        return m_voxelTypes.end();
    }

    DelegateSubscribers<std::span<RegisteredVoxelType> > &VoxelTypeRegistry::GetOnTypesRemovedDelegate() {
        return m_onTypesRemovedDelegate;
    }

    DelegateSubscribers<> &VoxelTypeRegistry::GetOnTypesChangedDelegate() {
        return m_onTypesChangedDelegate;
    }

    std::span<RegisteredVoxelType> VoxelTypeRegistry::GetTypes() {
        return std::span{m_voxelTypes};
    }

    Spire::Descriptor VoxelTypeRegistry::GetVoxelTypesBufferDescriptor(glm::u32 binding) {
        return Spire::Descriptor{
            .ResourceType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .Binding = binding,
            .Stages = VK_SHADER_STAGE_FRAGMENT_BIT,
            .Resources = {{.Buffer = &m_voxelTypesBuffer}},
#ifndef NDEBUG
            .DebugName = "Voxel Types Buffer"
#endif
        };
    }

    void VoxelTypeRegistry::RecreateVoxelTypesBuffer() {
        DestroyVoxelTypesBuffer();

        // how big does the buffer need to be
        glm::u32 numElements = 0;
        for (RegisteredVoxelType &type : m_voxelTypes) {
            numElements = std::max(numElements, type.GetType().Id + 1);
        }

        // prepare data
        std::vector<GPUVoxelType> data(numElements);
        for (RegisteredVoxelType &type : m_voxelTypes) {
            assert(type.FirstImageIndex != UINT32_MAX);
            data[type.GetType().Id] = {
                .FirstTextureIndex = type.FirstImageIndex,
                .VoxelFaceLayout = type.GetType().VoxelFaceLayout
            };
        }

        // create buffer
        m_voxelTypesBuffer = m_renderingManager.GetBufferManager().CreateStorageBuffer(data.data(), numElements * sizeof(GPUVoxelType), sizeof(GPUVoxelType));
    }

    void VoxelTypeRegistry::DestroyVoxelTypesBuffer() {
        if (m_voxelTypesBuffer.Buffer != VK_NULL_HANDLE) {
            m_renderingManager.GetBufferManager().DestroyBuffer(m_voxelTypesBuffer);
            m_voxelTypesBuffer.Buffer = VK_NULL_HANDLE;
        }
    }
} // SpireVoxel
