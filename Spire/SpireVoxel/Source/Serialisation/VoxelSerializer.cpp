#include "VoxelSerializer.h"

#include "Chunk/Chunk.h"
#include "Chunk/VoxelWorld.h"
#include "Rendering/VoxelWorldRenderer.h"
#include "Utils/FileIO.h"

namespace SpireVoxel {
    void VoxelSerializer::Serialize(VoxelWorld &world, const std::filesystem::path &directory) {
        for (const auto &[_, chunk] : world) {
            SerializeChunk(*chunk, directory);
        }

        Spire::info("Saved {} chunks to {}", world.NumLoadedChunks(), directory.string());
    }

    void VoxelSerializer::SerializeChunk(const Chunk &chunk, const std::filesystem::path &directory) {
        if (chunk.LOD.Scale != 1) {
            Spire::error("Attempted to serialize chunk {} {} {} but its LOD was {}", chunk.ChunkPosition.x, chunk.ChunkPosition.y, chunk.ChunkPosition.z, chunk.LOD.Scale);
            assert(false);
            return;
        }

        if (!std::filesystem::exists(directory) && !std::filesystem::create_directories(directory)) {
            Spire::error("Failed to create directory {} to serialize world");
            return;
        }

        std::filesystem::path path = GetFilePath(chunk.ChunkPosition, directory);
        std::ofstream file(path, std::ios_base::binary | std::ios_base::trunc);

        file.write(HEADER_IDENTIFIER.data(), HEADER_IDENTIFIER.size());
        file.write(reinterpret_cast<const char *>(&VERSION), sizeof(VERSION));
        file.write(reinterpret_cast<const char *>(&chunk.ChunkPosition), sizeof(chunk.ChunkPosition));
        file.write(reinterpret_cast<const char *>(chunk.VoxelData.data()), chunk.VoxelData.size() * sizeof(chunk.VoxelData[0]));

        file.close();
    }

    void VoxelSerializer::ClearAndDeserialize(
        VoxelWorld &world,
        const std::filesystem::path &directory
    ) {
        Spire::Timer timer;

        world.UnloadAllChunks();
        if (!std::filesystem::exists(directory)) return;

        std::unordered_set<glm::ivec3> loadedChunks;

        for (const auto &entry : std::filesystem::directory_iterator(directory)) {
            if (std::filesystem::is_directory(entry)) continue;
            if (Spire::FileIO::GetLowerCaseFileExtension(entry.path()) != ".sprc") continue;

            ChunkDeserialization result =                    ClearAndDeserializeChunkFile(world, entry.path());

            if (!result.Success) continue;

            if (!loadedChunks.insert(result.ChunkPos).second) {
                Spire::error(
                    "World at directory {} contains multiple files for chunk {}, {}, {}",
                    directory.string(),
                    result.ChunkPos.x,
                    result.ChunkPos.y,
                    result.ChunkPos.z
                );
            }
        }

        world.GetRenderer().HandleChunkEdits({});
        Spire::info("Deserialized world in {} ms", timer.MillisSinceStart());
    }

    VoxelSerializer::ChunkDeserialization VoxelSerializer::ClearAndDeserializeChunk(
        VoxelWorld &world,
        glm::ivec3 chunkCoords,
        const std::filesystem::path &directory) {
        return ClearAndDeserializeChunkFile(world, GetFilePath(chunkCoords,directory));
    }


    VoxelSerializer::ChunkDeserialization VoxelSerializer::ClearAndDeserializeChunkFile(
        VoxelWorld &world,
        const std::filesystem::path &filePath
    ) {
        ChunkDeserialization result{};
        result.DidChunkFileExist = true;

        std::ifstream file(filePath, std::ios_base::binary);
        if (!file) return result;

        std::remove_const_t<decltype(HEADER_IDENTIFIER)> identifier;
        if (!file.read(identifier.data(), identifier.size()) || identifier != HEADER_IDENTIFIER) {
            Spire::error("Failed to read .sprc file - invalid identifier {}", filePath.string());
            return result;
        }

        std::uint32_t version{};
        if (!file.read(reinterpret_cast<char *>(&version), sizeof(version)) || version > VERSION) {
            Spire::error("Failed to read version of {}", filePath.string());
            return result;
        }

        if (!file.read(reinterpret_cast<char *>(&result.ChunkPos), sizeof(result.ChunkPos))) {
            Spire::error("Failed to read chunk position of {}", filePath.string());
            return result;
        }

        Chunk &chunk = world.LoadChunk(result.ChunkPos);
        assert(!chunk.IsCorrupted());

        if (version < VOXEL_TYPE_U32_TO_U16_VERSION) {
            std::vector<std::uint32_t> legacy(chunk.VoxelData.size());
            if (!file.read(reinterpret_cast<char *>(legacy.data()), legacy.size() * sizeof(std::uint32_t))) {
                Spire::error("Failed to read legacy voxel data of {}", filePath.string());
                return result;
            }

            for (std::size_t i = 0; i < legacy.size(); ++i) {
                chunk.VoxelData[i] = static_cast<std::uint16_t>(legacy[i]);
            }

            result.Migrated = true;

            Spire::info(
                "Migrated chunk {} {} {} to latest voxel format",
                result.ChunkPos.x, result.ChunkPos.y, result.ChunkPos.z
            );
        } else {
            if (!file.read(reinterpret_cast<char *>(chunk.VoxelData.data()),
                           chunk.VoxelData.size() * sizeof(chunk.VoxelData[0]))) {
                Spire::error(
                    "Failed to read chunk voxel data of {} (chunk {} {} {})",
                    filePath.string(),
                    result.ChunkPos.x, result.ChunkPos.y, result.ChunkPos.z
                );
                return result;
            }
        }

        chunk.RegenerateVoxelBits();
        assert(!chunk.IsCorrupted());

        world.GetRenderer().NotifyChunkEdited(chunk);
        for (glm::u32 face = 0; face < SPIRE_VOXEL_NUM_FACES; face++) {
            Chunk *adjacent = world.TryGetLoadedChunk(chunk.ChunkPosition + FaceToDirection(face));
            if (adjacent) world.GetRenderer().NotifyChunkEdited(*adjacent);
        }

        if (result.Migrated) {
            SerializeChunk(chunk, filePath.parent_path());
        }

        result.Success = true;
        return result;
    }

    std::filesystem::path VoxelSerializer::GetFilePath(glm::ivec3 chunkCoords, const std::filesystem::path &directory) {
        return directory / std::format("{}_{}_{}.sprc", chunkCoords.x, chunkCoords.y, chunkCoords.z);
    }
} // SpireVoxel
