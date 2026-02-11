#include "SerializedGenerationProvider.h"
#include "VoxelSerializer.h"

SpireVoxel::SerializedGenerationProvider::SerializedGenerationProvider(
    const std::filesystem::path &path,
    std::unique_ptr<IProceduralGenerationProvider> backup
)
    : m_path(path),
      m_backupProvider(std::move(backup)) {
    assert(m_backupProvider); // Use EmptyProceduralGenerationProvider if you don't want one
}

void SpireVoxel::SerializedGenerationProvider::GenerateChunk(VoxelWorld &world, Chunk &chunk) {
    auto result = VoxelSerializer::ClearAndDeserializeChunk(world, chunk.ChunkPosition, m_path);
    if (!result.DidChunkFileExist) {
        m_backupProvider->GenerateChunk(world, chunk);
    }
}
