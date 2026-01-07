#include "GPUChunkMesher.h"

#include "Chunk/Chunk.h"
#include "../GreedyMeshingGrid.h"
#include "Rendering/Memory/VulkanAllocator.h"

namespace SpireVoxel {
    GPUChunkMesher::GPUChunkMesher(Spire::RenderingManager &renderingManager)
        : m_renderingManager(renderingManager) {
    }

    GPUChunkMesher::~GPUChunkMesher() {
        m_renderingManager.GetCommandManager().FreeCommandBuffers(m_commandBuffers.size(), m_commandBuffers.data());
    }

    void GPUChunkMesher::OnInit() {
        m_commandBuffers.resize(GetMaxParallelisation());
        m_renderingManager.GetCommandManager().CreateCommandBuffers(m_commandBuffers.size(), m_commandBuffers.data());

    }


    std::vector<VertexData> GPUChunkMesher::GenerateMesh(const std::shared_ptr<Chunk> &chunk, glm::u32 threadId) {
        auto &rm = m_renderingManager;

        static float irrelevantMillis = 0;
        static float runMillis = 0;
        static float mergeMillis = 0;
        static int numChunks = 0;
        numChunks++;
        Spire::Timer timer;

        // create buffers
        Spire::VulkanBuffer voxelDataBuffer = rm.GetBufferManager().CreateStorageBuffer(chunk->VoxelData.data(), sizeof(chunk->VoxelData[0]) * chunk->VoxelData.size(),
                                                                                        sizeof(chunk->VoxelData[0]));
        std::vector<glm::u64> shaderOutput(SPIRE_VOXEL_CHUNK_SIZE * SPIRE_VOXEL_CHUNK_AREA * SPIRE_VOXEL_NUM_FACES);
        Spire::VulkanBuffer outputBuffer = rm.GetBufferManager().CreateStorageBuffer(shaderOutput.data(), sizeof(shaderOutput[0]) * shaderOutput.size(),
                                                                                     sizeof(shaderOutput[0]),
                                                                                     true, VK_MEMORY_PROPERTY_HOST_CACHED_BIT /*  |VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT */);
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

        // create fences
        VkFence fence;
        VkFenceCreateInfo fenceInfo{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO
        };
        vkCreateFence(rm.GetDevice(), &fenceInfo, nullptr, &fence);

        // run
        VkCommandBuffer cmdBuffer = m_commandBuffers[threadId];
        rm.GetCommandManager().BeginCommandBuffer(cmdBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        descriptorManager.CmdBind(
            cmdBuffer,
            UINT32_MAX,
            pipeline.GetLayout(),
            SPIRE_VOXEL_SHADER_BINDINGS_GREEDY_MESHING_SET,
            SPIRE_VOXEL_SHADER_BINDINGS_GREEDY_MESHING_SET
        );

        pipeline.CmdBindTo(cmdBuffer);
        pipeline.CmdDispatch(cmdBuffer, 1, 1, 1);

        VkBufferMemoryBarrier2 computeToTransfer = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
            .srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            .dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer = outputBuffer.Buffer,
            .offset = 0,
            .size = VK_WHOLE_SIZE
        };

        VkDependencyInfo dependencyInfo = {
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .pNext = VK_NULL_HANDLE,
            .dependencyFlags = 0,
            .memoryBarrierCount = 0,
            .pMemoryBarriers = VK_NULL_HANDLE,
            .bufferMemoryBarrierCount = 1,
            .pBufferMemoryBarriers = &computeToTransfer,
            .imageMemoryBarrierCount = 0,
            .pImageMemoryBarriers = VK_NULL_HANDLE
        };

        vkCmdPipelineBarrier2(cmdBuffer, &dependencyInfo);

        rm.GetBufferManager().CmdCopyBuffer(cmdBuffer, outputBuffer, copyBuffer);

        VkBufferMemoryBarrier2 transferToHost = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            .srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_HOST_BIT,
            .dstAccessMask = VK_ACCESS_2_HOST_READ_BIT,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer = copyBuffer.Buffer,
            .offset = 0,
            .size = VK_WHOLE_SIZE
        };

        VkDependencyInfo dependencyInfo2 = {
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .pNext = VK_NULL_HANDLE,
            .dependencyFlags = 0,
            .memoryBarrierCount = 0,
            .pMemoryBarriers = VK_NULL_HANDLE,
            .bufferMemoryBarrierCount = 1,
            .pBufferMemoryBarriers = &transferToHost,
            .imageMemoryBarrierCount = 0,
            .pImageMemoryBarriers = VK_NULL_HANDLE
        };
        vkCmdPipelineBarrier2(cmdBuffer, &dependencyInfo2);

        rm.GetCommandManager().EndCommandBuffer(cmdBuffer);
        rm.GetQueue().Submit(cmdBuffer, fence);
        vkWaitForFences(rm.GetDevice(), 1, &fence, VK_TRUE, UINT64_MAX);

        // Read buffer into vector
        rm.GetBufferManager().ReadBufferElements(copyBuffer, shaderOutput);

        Spire::info("Compute + copy ran in {} ms", timer.MillisSinceStart());
        runMillis += timer.MillisSinceStart();
        timer.Restart();

        // Free resources
        vkDestroyFence(rm.GetDevice(), fence, nullptr);
        m_renderingManager.GetBufferManager().DestroyBuffer(outputBuffer);
        m_renderingManager.GetBufferManager().DestroyBuffer(copyBuffer);
        m_renderingManager.GetBufferManager().DestroyBuffer(voxelDataBuffer);
        vkDestroyShaderModule(rm.GetDevice(), shader, nullptr);
        rm.GetCommandManager().FreeCommandBuffers(1, &cmdBuffer);

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
        Spire::info("merge millis {}", mergeMillis);

        Spire::info("run millis average: {}", runMillis / static_cast<float>(numChunks));
        Spire::info("merge millis average: {}", mergeMillis / static_cast<float>(numChunks));

        return vertices;
    }

    float GPUChunkMesher::GetLoad() {
        return 1; // todo
    }

    glm::u32 GPUChunkMesher::GetMaxParallelisation() const {
        return 5; // todo
    }
} // SpireVoxel
