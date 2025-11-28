#include "VoxelSerializer.h"

#include "Chunk/Chunk.h"
#include "Chunk/VoxelWorld.h"
#include "Rendering/VoxelWorldRenderer.h"
#include "Utils/FileIO.h"

namespace SpireVoxel {
    void VoxelSerializer::Serialize(VoxelWorld &world, const std::filesystem::path &directory) {
        if (!std::filesystem::exists(directory) && !std::filesystem::create_directories(directory)) {
            Spire::error("Failed to create directory {} to serialize world");
            return;
        }

        for (const auto &[_, chunk] : world) {
            std::filesystem::path path = directory / std::format("{}_{}_{}.sprc", chunk->ChunkPosition.x, chunk->ChunkPosition.y, chunk->ChunkPosition.z);
            std::ofstream file(path, std::ios_base::binary | std::ios_base::trunc);

            file.write(HEADER_IDENTIFIER.data(), HEADER_IDENTIFIER.size());
            file.write(reinterpret_cast<const char *>(&VERSION), sizeof(VERSION));
            file.write(reinterpret_cast<const char *>(&chunk->ChunkPosition), sizeof(chunk->ChunkPosition));
            file.write(reinterpret_cast<const char *>(chunk->VoxelData.data()), chunk->VoxelData.size() * sizeof(chunk->VoxelData[0]));

            file.close();
        }

        Spire::info("Saved {} chunks to {}", world.GetNumLoadedChunks(), directory.string());
    }

    void VoxelSerializer::ClearAndDeserialize(VoxelWorld &world, const std::filesystem::path &directory) {
Spire::Timer timer;

        world.UnloadAllChunks();

        if (!std::filesystem::exists(directory)) return;

        std::unordered_set<glm::ivec3> loadedChunks;
        for (const auto &entry : std::filesystem::directory_iterator(directory)) {
            if (std::filesystem::is_directory(entry)) continue;
            if (Spire::FileIO::GetLowerCaseFileExtension(entry.path()) != ".sprc") continue;

            std::ifstream file(entry.path(), std::ios_base::binary);

            // read identifier
            std::remove_const_t<decltype(HEADER_IDENTIFIER)> identifier;
            if (!file.read(identifier.data(), identifier.size()) || identifier != HEADER_IDENTIFIER) {
                std::string identifierString;
                for (auto &c : identifier) identifierString.push_back(c);
                Spire::error("Failed to read .sprc file - invalid identifier {}", identifierString);
                continue;
            }

            // version
            std::remove_const_t<decltype(VERSION)> version{};
            if (!file.read(reinterpret_cast<char *>(&version), sizeof(VERSION)) || version > VERSION) {
                Spire::error("Failed to read version of {}", entry.path().string());
                continue;
            }

            // chunk pos
            glm::ivec3 chunkPos = {};
            if (!file.read(reinterpret_cast<char *>(&chunkPos), sizeof(chunkPos))) {
                Spire::error("Failed to read chunk position of {}", entry.path().string());
                continue;
            }

            if (loadedChunks.contains(chunkPos)) {
                Spire::error("World at directory {} contains multiple files for chunk {}, {}, {}", entry.path().string(), chunkPos.x, chunkPos.y,
                             chunkPos.z);
                continue;
            }
            loadedChunks.insert(chunkPos);

            Chunk &chunk = world.LoadChunk(chunkPos);
            assert(!chunk.IsCorrupted());
            if (!file.read(reinterpret_cast<char *>(chunk.VoxelData.data()), chunk.VoxelData.size() * sizeof(chunk.VoxelData[0]))) {
                Spire::error("Failed to read chunk voxel data of {} (chunk {})", entry.path().string(), chunkPos.x, chunkPos.y, chunkPos.z);
                continue;
            }
            chunk.RegenerateVoxelBits();
            assert(!chunk.IsCorrupted());

            world.GetRenderer().NotifyChunkEdited(chunk);
            for (glm::u32 face = 0; face < SPIRE_VOXEL_NUM_FACES; face++) {
                Chunk *adjacent = world.GetLoadedChunk(chunk.ChunkPosition + FaceToDirection(face));
                if (adjacent) world.GetRenderer().NotifyChunkEdited(*adjacent);
            }
        }

        world.GetRenderer().HandleChunkEdits();
        Spire::info("Deserialized world in {} ms", timer.MillisSinceStart());
    }
} // SpireVoxel
