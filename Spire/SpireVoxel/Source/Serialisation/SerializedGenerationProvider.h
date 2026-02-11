#pragma once
#include "../Generation/Providers/IProceduralGenerationProvider.h"

// Loads data from disk, but uses another provider as a backup
namespace SpireVoxel {
    class SerializedGenerationProvider : public IProceduralGenerationProvider {
    public:
        SerializedGenerationProvider(const std::filesystem::path &path, std::unique_ptr<IProceduralGenerationProvider> backup);

    public:
        void GenerateChunk(VoxelWorld &world, Chunk &chunk) override;

    private:
        std::filesystem::path m_path;
        std::unique_ptr<IProceduralGenerationProvider> m_backupProvider;
    };
} // SpireVoxel
