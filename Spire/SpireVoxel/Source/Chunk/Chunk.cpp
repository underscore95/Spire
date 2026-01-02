#include "Chunk.h"

#include "GreedyMeshingGrid.h"
#include "VoxelWorld.h"

namespace SpireVoxel {
    glm::u32 GetAdjacentVoxelType(const Chunk &chunk, glm::ivec3 position, glm::u32 face) {
        glm::ivec3 queryChunkPosition = chunk.ChunkPosition;
        glm::ivec3 queryPosition = position + FaceToDirection(face);

        assert(queryPosition.x >= -1 && queryPosition.x <= SPIRE_VOXEL_CHUNK_SIZE);
        assert(queryPosition.y >= -1 && queryPosition.y <= SPIRE_VOXEL_CHUNK_SIZE);
        assert(queryPosition.z >= -1 && queryPosition.z <= SPIRE_VOXEL_CHUNK_SIZE);

        if (queryPosition.x < 0) {
            queryPosition.x += SPIRE_VOXEL_CHUNK_SIZE;
            queryChunkPosition.x--;
        }
        if (queryPosition.x >= SPIRE_VOXEL_CHUNK_SIZE) {
            queryPosition.x -= SPIRE_VOXEL_CHUNK_SIZE;
            queryChunkPosition.x++;
        }

        if (queryPosition.y < 0) {
            queryPosition.y += SPIRE_VOXEL_CHUNK_SIZE;
            queryChunkPosition.y--;
        }
        if (queryPosition.y >= SPIRE_VOXEL_CHUNK_SIZE) {
            queryPosition.y -= SPIRE_VOXEL_CHUNK_SIZE;
            queryChunkPosition.y++;
        }

        if (queryPosition.z < 0) {
            queryPosition.z += SPIRE_VOXEL_CHUNK_SIZE;
            queryChunkPosition.z--;
        }
        if (queryPosition.z >= SPIRE_VOXEL_CHUNK_SIZE) {
            queryPosition.z -= SPIRE_VOXEL_CHUNK_SIZE;
            queryChunkPosition.z++;
        }

        const Chunk *queryChunk = &chunk;
        if (queryChunkPosition != chunk.ChunkPosition) {
            queryChunk = chunk.World.GetLoadedChunk(queryChunkPosition);
            if (!queryChunk) return VOXEL_TYPE_AIR;
        }
        return queryChunk->VoxelData[SPIRE_VOXEL_POSITION_TO_INDEX(queryPosition)];
    }

    void Chunk::SetVoxel(glm::u32 index, glm::u32 type) {
        VoxelData[index] = type;
        VoxelBits[index] = static_cast<bool>(type);
    }

    void Chunk::SetVoxels(glm::u32 startIndex, glm::u32 endIndex, glm::u32 type) {
        std::fill(VoxelData.data() + startIndex, VoxelData.data() + endIndex, type);
        for (; startIndex <= endIndex; startIndex++) {
            VoxelBits[startIndex] = static_cast<bool>(type); // todo: can we do this in a single write?
        }
    }

    void PushFace(std::vector<VertexData> &vertices, glm::u32 face, glm::uvec3 p, glm::u32 width, glm::u32 height) {
        assert(width > 0);
        assert(height > 0);
        const glm::u32 w = width;
        const glm::u32 h = height;
        switch (face) {
            case SPIRE_VOXEL_FACE_POS_Z:
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + 0, p.z + 1, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_POS_Z));
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + h, p.z + 1, VoxelVertexPosition::THREE, SPIRE_VOXEL_FACE_POS_Z));
                vertices.push_back(PackVertexData(w, h, p.x + w, p.y + h, p.z + 1, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_POS_Z));
                vertices.push_back(PackVertexData(w, h, p.x + w, p.y + h, p.z + 1, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_POS_Z));
                vertices.push_back(PackVertexData(w, h, p.x + w, p.y + 0, p.z + 1, VoxelVertexPosition::ONE, SPIRE_VOXEL_FACE_POS_Z));
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + 0, p.z + 1, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_POS_Z));
                break;

            case SPIRE_VOXEL_FACE_NEG_Z:
                vertices.push_back(PackVertexData(w, h, p.x + w, p.y + 0, p.z + 0, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_NEG_Z));
                vertices.push_back(PackVertexData(w, h, p.x + w, p.y + h, p.z + 0, VoxelVertexPosition::THREE, SPIRE_VOXEL_FACE_NEG_Z));
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + h, p.z + 0, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_NEG_Z));
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + h, p.z + 0, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_NEG_Z));
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + 0, p.z + 0, VoxelVertexPosition::ONE, SPIRE_VOXEL_FACE_NEG_Z));
                vertices.push_back(PackVertexData(w, h, p.x + w, p.y + 0, p.z + 0, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_NEG_Z));
                break;

            case SPIRE_VOXEL_FACE_NEG_X:
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + 0, p.z + 0, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_NEG_X));
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + h, p.z + 0, VoxelVertexPosition::THREE, SPIRE_VOXEL_FACE_NEG_X));
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + h, p.z + w, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_NEG_X));
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + h, p.z + w, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_NEG_X));
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + 0, p.z + w, VoxelVertexPosition::ONE, SPIRE_VOXEL_FACE_NEG_X));
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + 0, p.z + 0, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_NEG_X));
                break;

            case SPIRE_VOXEL_FACE_POS_X:
                vertices.push_back(PackVertexData(w, h, p.x + 1, p.y + 0, p.z + w, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_POS_X));
                vertices.push_back(PackVertexData(w, h, p.x + 1, p.y + h, p.z + w, VoxelVertexPosition::THREE, SPIRE_VOXEL_FACE_POS_X));
                vertices.push_back(PackVertexData(w, h, p.x + 1, p.y + h, p.z + 0, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_POS_X));
                vertices.push_back(PackVertexData(w, h, p.x + 1, p.y + h, p.z + 0, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_POS_X));
                vertices.push_back(PackVertexData(w, h, p.x + 1, p.y + 0, p.z + 0, VoxelVertexPosition::ONE, SPIRE_VOXEL_FACE_POS_X));
                vertices.push_back(PackVertexData(w, h, p.x + 1, p.y + 0, p.z + w, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_POS_X));
                break;

            case SPIRE_VOXEL_FACE_POS_Y:
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + 1, p.z + h, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_POS_Y));
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + 1, p.z + 0, VoxelVertexPosition::THREE, SPIRE_VOXEL_FACE_POS_Y));
                vertices.push_back(PackVertexData(w, h, p.x + w, p.y + 1, p.z + 0, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_POS_Y));
                vertices.push_back(PackVertexData(w, h, p.x + w, p.y + 1, p.z + 0, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_POS_Y));
                vertices.push_back(PackVertexData(w, h, p.x + w, p.y + 1, p.z + h, VoxelVertexPosition::ONE, SPIRE_VOXEL_FACE_POS_Y));
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + 1, p.z + h, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_POS_Y));
                break;

            case SPIRE_VOXEL_FACE_NEG_Y:
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + 0, p.z + 0, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_NEG_Y));
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + 0, p.z + h, VoxelVertexPosition::THREE, SPIRE_VOXEL_FACE_NEG_Y));
                vertices.push_back(PackVertexData(w, h, p.x + w, p.y + 0, p.z + h, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_NEG_Y));
                vertices.push_back(PackVertexData(w, h, p.x + w, p.y + 0, p.z + h, VoxelVertexPosition::TWO, SPIRE_VOXEL_FACE_NEG_Y));
                vertices.push_back(PackVertexData(w, h, p.x + w, p.y + 0, p.z + 0, VoxelVertexPosition::ONE, SPIRE_VOXEL_FACE_NEG_Y));
                vertices.push_back(PackVertexData(w, h, p.x + 0, p.y + 0, p.z + 0, VoxelVertexPosition::ZERO, SPIRE_VOXEL_FACE_NEG_Y));
                break;
            default:
                assert(false);
                break;
        }
    }

    std::vector<VertexData> Chunk::GenerateMesh(Spire::RenderingManager &rm) const {
        static float irrelevantMillis = 0;
        static float runMillis = 0;
        static float readMillis = 0;
        static float mergeMillis = 0;
        static int numChunks = 0;
        numChunks++;
        Spire::Timer timer;

        // create buffers
        Spire::VulkanBuffer voxelDataBuffer = rm.GetBufferManager().CreateStorageBuffer(VoxelData.data(), sizeof(VoxelData[0]) * VoxelData.size(), sizeof(VoxelData[0]));
        std::vector<glm::u64> shaderOutput(SPIRE_VOXEL_CHUNK_SIZE * SPIRE_VOXEL_CHUNK_AREA * SPIRE_VOXEL_NUM_FACES);
        Spire::VulkanBuffer outputBuffer = rm.GetBufferManager().CreateStorageBuffer(shaderOutput.data(), sizeof(shaderOutput[0]) * shaderOutput.size(), sizeof(shaderOutput[0]),
                                                                                     true, VK_MEMORY_PROPERTY_HOST_CACHED_BIT);
        // output buffer will be copied since faster
        VkBufferUsageFlags copyBufferUsage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        VkMemoryPropertyFlags copyBufferProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
        Spire::VulkanBuffer copyBuffer = rm.GetBufferManager().CreateBuffer(outputBuffer.Size, copyBufferUsage, copyBufferProperties, outputBuffer.ElementSize);

        Spire::info("Created compute buffers in {} ms", timer.MillisSinceStart());
        irrelevantMillis += timer.MillisSinceStart();
        timer.Restart();

        // create shader
        Spire::ShaderCompiler compiler(rm.GetDevice());
        VkShaderModule shader = compiler.CreateShaderModule(std::format("{}/Shaders/GreedyMeshing.comp", ASSETS_DIRECTORY));

        // descriptors
        Spire::DescriptorSetLayoutList layouts(rm.GetSwapchain().GetNumImages());
        Spire::DescriptorSetLayout layout;
        layout.push_back(
            Spire::Descriptor{
                .ResourceType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .Binding = SPIRE_VOXEL_SHADER_BINDINGS_GREEDY_MESHING_VOXEL_DATA_INPUT,
                .Stages = VK_SHADER_STAGE_COMPUTE_BIT,
                .Resources = {{.Buffer = &voxelDataBuffer}},
#ifndef NDEBUG
                .DebugName = std::format("Voxel Data Compute"),
#endif
            }
        );

        layout.push_back(
            Spire::Descriptor{
                .ResourceType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .Binding = SPIRE_VOXEL_SHADER_BINDINGS_GREEDY_MESHING_OUTPUT,
                .Stages = VK_SHADER_STAGE_COMPUTE_BIT,
                .Resources = {{.Buffer = &outputBuffer}},
#ifndef NDEBUG
                .DebugName = std::format("Voxel Output Compute"),
#endif
            }
        );

        glm::u32 layoutIndex = layouts.Push(layout);
        assert(layoutIndex == SPIRE_VOXEL_SHADER_BINDINGS_GREEDY_MESHING_SET);
        Spire::DescriptorManager descriptorManager(rm, layouts, VK_PIPELINE_BIND_POINT_COMPUTE);

        // setup pipeline
        Spire::ComputePipeline pipeline(rm.GetDevice(), shader, descriptorManager, rm, 0);

        Spire::info("Compute pipeline setup in {} ms", timer.MillisSinceStart());
        irrelevantMillis += timer.MillisSinceStart();
        timer.Restart();

        // run
        VkCommandBuffer cmdBuffer;
        rm.GetCommandManager().CreateCommandBuffers(1, &cmdBuffer);
        rm.GetCommandManager().BeginCommandBuffer(cmdBuffer, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
        descriptorManager.CmdBind(cmdBuffer, UINT32_MAX, pipeline.GetLayout(),SPIRE_VOXEL_SHADER_BINDINGS_GREEDY_MESHING_SET,SPIRE_VOXEL_SHADER_BINDINGS_GREEDY_MESHING_SET);
        pipeline.CmdBindTo(cmdBuffer);
        pipeline.CmdDispatch(cmdBuffer, 1, 1, 1);
        rm.GetCommandManager().EndCommandBuffer(cmdBuffer);
        rm.GetQueue().SubmitImmediate(cmdBuffer);
        rm.GetQueue().WaitIdle();

        Spire::info("Compute shader ran in {} ms", timer.MillisSinceStart());
        runMillis += timer.MillisSinceStart();
        timer.Restart();

        // Read
        rm.GetBufferManager().CopyBuffer(copyBuffer.Buffer, outputBuffer.Buffer, copyBuffer.Size);
        rm.GetBufferManager().ReadBufferElements(copyBuffer, shaderOutput);
        Spire::info("Read shader output in {} ms", timer.MillisSinceStart());
        readMillis += timer.MillisSinceStart();
        timer.Restart();

        // greedy mesh on our grids
        std::vector<VertexData> vertices;
        for (glm::u32 face = 0; face < SPIRE_VOXEL_NUM_FACES; face++) {
            for (glm::u32 slice = 0; slice < SPIRE_VOXEL_CHUNK_SIZE; slice++) {
                assert(GreedyGridGetGridStartingIndex(face, slice) + SPIRE_VOXEL_CHUNK_SIZE <= shaderOutput.size());
                GreedyMeshingGrid grid(&shaderOutput[GreedyGridGetGridStartingIndex(face, slice)]);
                for (glm::i32 col = 0; col < SPIRE_VOXEL_CHUNK_SIZE; col++) {
                    // find the starting row and height of the face
                    if (grid.GetColumn(col) == 0) continue;
                    glm::u32 row = grid.NumTrailingEmptyVoxels(col, 0);
                    glm::u32 height = grid.NumTrailingPresentVoxels(col, row);

                    // absorb faces
                    grid.SetEmptyVoxels(col, row, height);

                    // move as far right as we can
                    glm::u32 width = 1;
                    while (col + width < SPIRE_VOXEL_CHUNK_SIZE && grid.NumTrailingPresentVoxels(col + width, row) >= height) {
                        grid.SetEmptyVoxels(col + width, row, height); // absorb the new column
                        width++;
                    }
                    //     mask.Print();

                    // push the face
                    glm::uvec3 chunkCoords = GreedyMeshingGrid::GetChunkCoords(slice, row, col, face);
                    PushFace(vertices, face, chunkCoords, width, height);

                    if (grid.GetColumn(col) != 0) {
                        // we didn't get all the voxels on this row, loop again
                        col--;
                    }
                }
            }
        }

        Spire::info("Merged shader output in {} ms", timer.MillisSinceStart());
        mergeMillis += timer.MillisSinceStart();

        Spire::info("irrelevant millis {}", irrelevantMillis);
        Spire::info("run millis {}", runMillis);
        Spire::info("read millis {}", readMillis);
        Spire::info("merge millis {}", mergeMillis);
        Spire::info("all millis {}", runMillis + readMillis + mergeMillis);

        Spire::info("run millis average: {}", runMillis / static_cast<float>(numChunks));
        Spire::info("read millis average: {}", readMillis / static_cast<float>(numChunks));
        Spire::info("merge millis average: {}", mergeMillis / static_cast<float>(numChunks));
        Spire::info("all millis average: {}", (runMillis + readMillis + mergeMillis) / static_cast<float>(numChunks));
        return vertices;
    }

    ChunkData Chunk::GenerateChunkData(glm::u32 chunkIndex) const {
        assert(NumVertices > 0);
        return {
            .CPU_DrawCommandParams = {
                .vertexCount = (NumVertices),
                .instanceCount = 1,
                .firstVertex = static_cast<glm::u32>(VertexAllocation.Start / sizeof(VertexData)),
                .firstInstance = chunkIndex
            },
            .ChunkX = ChunkPosition.x,
            .ChunkY = ChunkPosition.y,
            .ChunkZ = ChunkPosition.z,
            .VoxelDataChunkIndex = static_cast<glm::u32>(VoxelDataAllocation.Start / sizeof(GPUChunkVoxelData))
        };
    }

    void Chunk::RegenerateVoxelBits() {
        for (std::size_t i = 0; i < VoxelData.size(); i++) {
            VoxelBits[i] = static_cast<bool>(VoxelData[i]); // todo can this be done in a single write?
        }
    }

    std::optional<std::size_t> Chunk::GetIndexOfVoxel(glm::ivec3 chunkPosition, glm::ivec3 voxelWorldPosition) {
        if (chunkPosition != VoxelWorld::GetChunkPositionOfVoxel(voxelWorldPosition)) return std::nullopt;
        glm::uvec3 pos = voxelWorldPosition - chunkPosition * SPIRE_VOXEL_CHUNK_SIZE;
        return {SPIRE_VOXEL_POSITION_TO_INDEX(pos)};
    }
} // SpireVoxel
