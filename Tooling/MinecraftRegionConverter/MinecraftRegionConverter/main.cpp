#include <array>
#include <iostream>
#include <vector>
#include <mutex>
#include <thread>
#include <atomic>
#include <memory>
#include <chrono>
#include <filesystem>
#include <fstream>
#include "minecraft.h"

#include "enkimi.h"

// https://github.com/dougbinks/enkiMI/blob/master/example/example.c
// https://github.com/dougbinks/enkiMI/blob/master/README.md

constexpr int CHUNK_SIZE = 64;
constexpr int REGION_SIZE_X = 512;
constexpr int REGION_SIZE_Y = 384;
constexpr int REGION_SIZE_Y_START_CHUNK = -1;
constexpr int REGION_SIZE_Z = REGION_SIZE_X;

struct SpireChunk {
    std::array<uint16_t, CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE> VoxelTypes;
};

template<>
struct std::hash<enkiMICoordinate> {
    std::size_t operator()(const enkiMICoordinate &s) const noexcept {
        std::size_t h1 = std::hash<int32_t>{}(s.x);
        std::size_t h2 = std::hash<int32_t>{}(s.y);
        std::size_t h3 = std::hash<int32_t>{}(s.z);
        return (h1 ^ (h2 << 1)) ^ (h3 << 1);
    }
};


#define SPIRE_VOXEL_POSITION_XYZ_TO_INDEX(x, y, z) \
((x) * (CHUNK_SIZE*CHUNK_SIZE) + (y) * CHUNK_SIZE + (z))

#define SPIRE_VOXEL_POSITION_TO_INDEX(pos) SPIRE_VOXEL_POSITION_XYZ_TO_INDEX(pos.x, pos.y, pos.z)

bool operator==(const enkiMICoordinate &lhs, const enkiMICoordinate &rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

enkiMICoordinate operator%(const enkiMICoordinate &lhs, int rhs) {
    return {lhs.x % rhs, lhs.y % rhs, lhs.z % rhs};
}

enkiMICoordinate GetChunkCoordFromIndex(int index) {
    const int y = index / ((REGION_SIZE_X / CHUNK_SIZE) * (REGION_SIZE_Z / CHUNK_SIZE));
    const int remainder = index % ((REGION_SIZE_X / CHUNK_SIZE) * (REGION_SIZE_Z / CHUNK_SIZE));

    const int z = remainder / (REGION_SIZE_X / CHUNK_SIZE);
    const int x = remainder % (REGION_SIZE_X / CHUNK_SIZE);

    return {
        x,
        y + REGION_SIZE_Y_START_CHUNK,
        z
    };
}

int GetChunkIndex(enkiMICoordinate chunkCoord) {
    const int x = chunkCoord.x;
    const int y = chunkCoord.y - REGION_SIZE_Y_START_CHUNK;
    const int z = chunkCoord.z;

    return x + (z * REGION_SIZE_X / CHUNK_SIZE) + (y * REGION_SIZE_X / CHUNK_SIZE * REGION_SIZE_Z / CHUNK_SIZE);
}

// copied from spire
void SerializeChunk(const SpireChunk &chunk, enkiMICoordinate chunkCoords, const std::filesystem::path &directory) {
    static constexpr std::array<char, 10> HEADER_IDENTIFIER{'S', 'P', 'R', 'V', 'X', 'L', 'C', 'H', 'N', 'K'};
    static constexpr std::uint32_t VERSION = 2;

    std::filesystem::path path = directory / std::format("{}_{}_{}.sprc", chunkCoords.x, chunkCoords.y, chunkCoords.z);
    std::ofstream file(path, std::ios_base::binary | std::ios_base::trunc);

    file.write(HEADER_IDENTIFIER.data(), HEADER_IDENTIFIER.size());
    file.write(reinterpret_cast<const char *>(&VERSION), sizeof(VERSION));
    file.write(reinterpret_cast<const char *>(&chunkCoords), sizeof(chunkCoords));
    file.write(reinterpret_cast<const char *>(chunk.VoxelTypes.data()), chunk.VoxelTypes.size() * sizeof(chunk.VoxelTypes[0]));

    file.close();
}


int main(int argc, const char **argv) {
    if (argc < 5) {
        std::cerr << "Invalid usage! Arugments: input region file, output directory, minecraft dir, types output file [-overwrite]" << "\n";
        return -1;
    }
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

    const char *input = argv[1];
    const char *output = argv[2];
    std::filesystem::path outputDir(output);
    std::filesystem::path minecraftDir(argv[3]); // Extract Minecraft jar (%appdata%/.minecraft/versions) and place it into this directory
    std::filesystem::path typesOutputFile(argv[4]);

    bool overwriteOutput = argc > 5;
    if (overwriteOutput && argv[5] != std::string("-overwrite")) {
        std::cerr << "Invalid argument " << argv[5] << ", expected -overwrite\n";
        return -1;
    }

    std::unordered_map<std::string, int> types;
    if (!LoadTypes(minecraftDir, typesOutputFile, overwriteOutput, types)) return -1;

    FILE *fp = fopen(input, "rb");

    if (!fp) {
        std::cerr << "Failed to open input file\n";
        return -2;
    }

    // delete and create dir
    if (std::filesystem::exists(outputDir)) {
        if (overwriteOutput) {
            if (!std::filesystem::remove_all(outputDir)) {
                std::cerr << "Failed to delete directory " << outputDir.string() << "\n";
            }
        } else {
            std::cerr << std::format("Output directory {} already exists\n", outputDir.string());
            return -3;
        }
    }
    if (!std::filesystem::create_directories(outputDir)) {
        std::cerr << std::format("Failed to create directory {} to serialize world\n", outputDir.string());
        return -4;
    }

    log(std::format("Converting Minecraft region {} into a Spire world (directory {})...", input, outputDir.string()));

    using ChunkArray = std::array<
        SpireChunk,
        (REGION_SIZE_X / CHUNK_SIZE) *
        (REGION_SIZE_Y / CHUNK_SIZE) *
        (REGION_SIZE_Z / CHUNK_SIZE)
    >;

    std::unique_ptr<ChunkArray> chunks = std::make_unique<ChunkArray>();
    enkiRegionFile regionFile = enkiRegionFileLoad(fp);

    // dispatch threads
    int numThreads = std::thread::hardware_concurrency();
    log(std::format("Using {} threads...", numThreads));
    int chunksPerThread = std::ceil(ENKI_MI_REGION_CHUNKS_NUMBER / static_cast<float>(numThreads));
    std::atomic_int processedChunks = 0;
    std::vector<std::thread> threadVector;
    for (int thread = 0; thread < numThreads; thread++) {
        threadVector.emplace_back([chunksPerThread, thread, &regionFile, &chunks, &processedChunks]() {
            // loop over all chunks this thread is responsible for
            for (int i = chunksPerThread * thread; i < (thread + 1) * chunksPerThread && i < ENKI_MI_REGION_CHUNKS_NUMBER; i++) {
                processedChunks += 1;
                enkiNBTDataStream stream;
                enkiInitNBTDataStreamForChunk(regionFile, i, &stream);
                if (stream.dataLength) {
                    enkiChunkBlockData aChunk = enkiNBTReadChunk(&stream);
                    for (int sectionIndex = 0; sectionIndex < ENKI_MI_NUM_SECTIONS_PER_CHUNK; sectionIndex++) {
                        uint8_t *section = aChunk.sections[sectionIndex];
                        if (!section) continue;

                        enkiMICoordinate sectionOrigin = enkiGetChunkSectionOrigin(&aChunk, sectionIndex);

                        // loop sections in chunk
                        enkiMICoordinate sPos;
                        for (sPos.y = 0; sPos.y < ENKI_MI_SIZE_SECTIONS; ++sPos.y) {
                            for (sPos.z = 0; sPos.z < ENKI_MI_SIZE_SECTIONS; ++sPos.z) {
                                for (sPos.x = 0; sPos.x < ENKI_MI_SIZE_SECTIONS; ++sPos.x) {
                                    enkiMICoordinate worldPos = {
                                        sectionOrigin.x + sPos.x,
                                        sectionOrigin.y + sPos.y,
                                        sectionOrigin.z + sPos.z
                                    };

                                    // get voxel id
                                    enkiMIVoxelData voxelData = enkiGetChunkSectionVoxelData(&aChunk, sectionIndex, sPos);
                                    std::string id = "minecraft:air";
                                    if (voxelData.paletteIndex >= 0) {
                                        auto [size, pStrNotNullTerminated] = aChunk.palette[sectionIndex].pNamespaceIDStrings[voxelData.paletteIndex];

                                        id = {pStrNotNullTerminated, size};
                                    }

                                    // convert to spire id
                                    int type = 0;
                                    if (id == "minecraft:grass_block") {
                                        type = 1;
                                    } else if (id != "minecraft:air") {
                                        type = 2;
                                    }

                                    // calculate index to write
                                    // 54 -> 54
                                    // 68 -> 4
                                    // -68 -> 60
                                    enkiMICoordinate chunkCoord = {
                                        (worldPos.x % CHUNK_SIZE) + (worldPos.x % CHUNK_SIZE < 0 ? CHUNK_SIZE : 0),
                                        (worldPos.y % CHUNK_SIZE) + (worldPos.y % CHUNK_SIZE < 0 ? CHUNK_SIZE : 0),
                                        (worldPos.z % CHUNK_SIZE) + (worldPos.z % CHUNK_SIZE < 0 ? CHUNK_SIZE : 0)
                                    };
                                    assert(chunkCoord.x >= 0);
                                    assert(chunkCoord.y >= 0);
                                    assert(chunkCoord.z >= 0);
                                    assert(chunkCoord.x <CHUNK_SIZE);
                                    assert(chunkCoord.y <CHUNK_SIZE);
                                    assert(chunkCoord.z <CHUNK_SIZE);
                                    std::size_t index = SPIRE_VOXEL_POSITION_TO_INDEX(chunkCoord);
                                    assert(index < CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE);

                                    // get chunk
                                    // floor div
                                    int chunkIndex = GetChunkIndex({
                                        worldPos.x >= 0 ? worldPos.x / CHUNK_SIZE : (worldPos.x - (CHUNK_SIZE - 1)) / CHUNK_SIZE,
                                        worldPos.y >= 0 ? worldPos.y / CHUNK_SIZE : (worldPos.y - (CHUNK_SIZE - 1)) / CHUNK_SIZE,
                                        worldPos.z >= 0 ? worldPos.z / CHUNK_SIZE : (worldPos.z - (CHUNK_SIZE - 1)) / CHUNK_SIZE
                                    });
                                    assert(chunkIndex >= 0 && chunkIndex<chunks->size());
                                    SpireChunk &spireChunk = (*chunks)[chunkIndex];

                                    // write
                                    // this is safe because we only ever write to each index once in each chunk, no need for
                                    // any synchronization here
                                    spireChunk.VoxelTypes[index] = type;
                                }
                            }
                        }
                    }
                }
                enkiNBTFreeAllocations(&stream);
            }
        });
    }

    // wait threads
    for (std::thread &t : threadVector) {
        t.join();
    }
    threadVector.clear();

    // cleanup enki
    enkiRegionFileFreeAllocations(&regionFile);
    fclose(fp);

    // write
    chunksPerThread = chunks->size() / numThreads;
    for (int i = 0; i < numThreads; i++) {
        threadVector.emplace_back([chunksPerThread, i, outputDir, &chunks]() {
            for (int chunkIndex = (chunksPerThread * i); chunkIndex < (i + 1) * chunksPerThread; chunkIndex++) {
                enkiMICoordinate chunkCoord = GetChunkCoordFromIndex(chunkIndex);
                SerializeChunk((*chunks)[chunkIndex], chunkCoord, outputDir);
            }
        });
    }

    // wait threads
    for (std::thread &t : threadVector) {
        t.join();
    }
    threadVector.clear();

    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
    std::cout << "Converted on " << numThreads << " threads in " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms\n";
}
