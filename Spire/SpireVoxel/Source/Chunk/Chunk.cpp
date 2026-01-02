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

    std::vector<VertexData> Chunk::GenerateMesh(Spire::RenderingManager &rm) const {
        static float runMillis=0;
        static float readMillis=0;
        static float mergeMillis=0;
        Spire::Timer timer;

        // create buffers
        Spire::VulkanBuffer voxelDataBuffer = rm.GetBufferManager().CreateStorageBuffer(VoxelData.data(), sizeof(VoxelData[0]) * VoxelData.size(), sizeof(VoxelData[0]));
        std::vector<GreedyMeshOutput> shaderOutput(SPIRE_VOXEL_CHUNK_SIZE * SPIRE_VOXEL_NUM_FACES);
        Spire::VulkanBuffer outputBuffer = rm.GetBufferManager().CreateStorageBuffer(shaderOutput.data(), sizeof(GreedyMeshOutput) * shaderOutput.size(), sizeof(GreedyMeshOutput));

        Spire::info("Created compute buffers in {} ms", timer.MillisSinceStart());
        timer.Restart();

        // create shader
        Spire::ShaderCompiler compiler(rm.GetDevice());
        Spire::ShaderCompiler::Options options = {
            .Optimise = false,
            .OptimiseSize = false
        };
        VkShaderModule shader = compiler.CreateShaderModule(std::format("{}/Shaders/GreedyMeshing.comp", ASSETS_DIRECTORY), options);

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
        runMillis+=timer.MillisSinceStart();
        timer.Restart();

        rm.GetBufferManager().ReadBufferElements(outputBuffer, shaderOutput);
        Spire::info("Read shader output in {} ms", timer.MillisSinceStart());
        readMillis+=timer.MillisSinceStart();
        timer.Restart();

        // merge output into a single vector
        std::vector<VertexData> vertices;
        for (const GreedyMeshOutput &output : shaderOutput) {
          //  Spire::info("num: {}", output.NumVertices);
            for (int i = 0; i < output.NumVertices; i++) {
             vertices.push_back(output.Vertices[i]);
            }
        }

   //     Spire::info("num verticeS: {}",vertices.size());
        Spire::info("Merged shader output in {} ms", timer.MillisSinceStart());
        mergeMillis+=timer.MillisSinceStart();
        timer.Restart();
        static int numChunks=0;
        numChunks++;
Spire::info("total run: {} read: {} merge: {} (millis), total chunks {}",runMillis,readMillis,mergeMillis,numChunks);
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
