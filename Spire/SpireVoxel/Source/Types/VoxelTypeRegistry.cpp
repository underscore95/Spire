#include "VoxelTypeRegistry.h"

#include "RegisteredVoxelType.h"
#include "VoxelType.h"
#include "Chunk/Chunk.h"

namespace SpireVoxel {
    VoxelTypeRegistry::VoxelTypeRegistry() : m_onTypesRemovedDelegate(), m_onTypesChangedDelegate() {
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
} // SpireVoxel
