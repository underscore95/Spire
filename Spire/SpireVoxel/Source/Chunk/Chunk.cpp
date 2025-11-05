#include "Chunk.h"

namespace SpireVoxel {
    void Chunk::SetVoxel(glm::vec3 pos, std::int32_t type) {
        m_voxelData[SPIRE_VOXEL_POSITION_TO_INDEX(pos)] = type;
    }

    void Chunk::SetVoxelRect(glm::vec3 pos, glm::vec3 rectDimensions, std::int32_t type) {
        for (size_t x = 0; x < rectDimensions.x; ++x) {
            for (size_t y = 0; y < rectDimensions.y; ++y) {
                for (size_t z = 0; z < rectDimensions.z; ++z) {
                    SetVoxel(pos + glm::vec3{x, y, z}, type); // todo: optimise
                }
            }
        }
    }

    Spire::Model Chunk::GenerateModel() {
        Spire::Model model = {};
        model.push_back(std::make_unique<Spire::Mesh>());
        auto &mesh = *model.back();

        mesh.ImageIndex = 0;

        for (size_t i = 0; i < m_voxelData.size(); i++) {
            if (m_voxelData[i] == 0) continue;

            glm::vec3 p = SPIRE_VOXEL_INDEX_TO_POSITION(glm::vec3, i);
            uint32_t start = static_cast<uint32_t>(mesh.Vertices.size());

            mesh.Vertices.push_back(Spire::ModelVertex{p + glm::vec3(0, 0, 0), glm::vec3(0, 0, 0)});
            mesh.Vertices.push_back(Spire::ModelVertex{p + glm::vec3(1, 0, 0), glm::vec3(1, 0, 0)});
            mesh.Vertices.push_back(Spire::ModelVertex{p + glm::vec3(1, 1, 0), glm::vec3(1, 1, 0)});
            mesh.Vertices.push_back(Spire::ModelVertex{p + glm::vec3(0, 1, 0), glm::vec3(0, 1, 0)});

            mesh.Vertices.push_back(Spire::ModelVertex{p + glm::vec3(0, 0, 1), glm::vec3(0, 0, 1)});
            mesh.Vertices.push_back(Spire::ModelVertex{p + glm::vec3(1, 0, 1), glm::vec3(1, 0, 1)});
            mesh.Vertices.push_back(Spire::ModelVertex{p + glm::vec3(1, 1, 1), glm::vec3(1, 1, 1)});
            mesh.Vertices.push_back(Spire::ModelVertex{p + glm::vec3(0, 1, 1), glm::vec3(0, 1, 1)});

            mesh.Indices.push_back(start + 0);
            mesh.Indices.push_back(start + 1);
            mesh.Indices.push_back(start + 2);
            mesh.Indices.push_back(start + 2);
            mesh.Indices.push_back(start + 3);
            mesh.Indices.push_back(start + 0);

            mesh.Indices.push_back(start + 4);
            mesh.Indices.push_back(start + 5);
            mesh.Indices.push_back(start + 6);
            mesh.Indices.push_back(start + 6);
            mesh.Indices.push_back(start + 7);
            mesh.Indices.push_back(start + 4);

            mesh.Indices.push_back(start + 0);
            mesh.Indices.push_back(start + 4);
            mesh.Indices.push_back(start + 7);
            mesh.Indices.push_back(start + 7);
            mesh.Indices.push_back(start + 3);
            mesh.Indices.push_back(start + 0);

            mesh.Indices.push_back(start + 1);
            mesh.Indices.push_back(start + 5);
            mesh.Indices.push_back(start + 6);
            mesh.Indices.push_back(start + 6);
            mesh.Indices.push_back(start + 2);
            mesh.Indices.push_back(start + 1);

            mesh.Indices.push_back(start + 3);
            mesh.Indices.push_back(start + 2);
            mesh.Indices.push_back(start + 6);
            mesh.Indices.push_back(start + 6);
            mesh.Indices.push_back(start + 7);
            mesh.Indices.push_back(start + 3);

            mesh.Indices.push_back(start + 0);
            mesh.Indices.push_back(start + 1);
            mesh.Indices.push_back(start + 5);
            mesh.Indices.push_back(start + 5);
            mesh.Indices.push_back(start + 4);
            mesh.Indices.push_back(start + 0);
        }

        return std::move(model);
    }
} // SpireVoxel
