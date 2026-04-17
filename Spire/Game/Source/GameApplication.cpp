#include "GameApplication.h"
#include "../../Libs/glfw/include/GLFW/glfw3.h"
#include "GameCamera.h"
#include "MinecraftTypes.h"
#include "Profiling.h"
#include "../../SpireVoxel/Source/ChunkOrderControllers/EmptyChunkOrderController.h"
#include "Generation/Controllers/SimpleProceduralGenerationController.h"
#include "Generation/Providers/EmptyProceduralGenerationProvider.h"
#include "../../SpireVoxel/Source/Serialisation/SerializedGenerationProvider.h"
#include "Generation/Providers/SimpleProceduralGenerationProvider.h"
#include "LOD/SamplingOffsets.h"
#include "Serialisation/VoxelSerializer.h"
#include "Types/VoxelTypeInfo.h"
#include "Types/VoxelTypeRegistry.h"
#include "Utils/RaycastUtils.h"

using namespace Spire;
using namespace SpireVoxel;

bool ShouldStreamLoading() {
    if (Profiling::IS_PROFILING) return false;
    return false;
    return true;
}

GameApplication::GameApplication() = default;

constexpr const char* WORLD_NAME = Profiling::IS_PROFILING ? Profiling::PROFILE_WORLD_NAME : "Test3";

void GameApplication::Start(Engine &engine) {
    m_engine = &engine;

    std::unique_ptr<IChunkOrderController> proceduralGenerationController = std::make_unique<EmptyChunkOrderController>();
    std::unique_ptr<IProceduralGenerationProvider> proceduralGenerationProvider = std::make_unique<EmptyProceduralGenerationProvider>();
    if (ShouldStreamLoading()) {
        proceduralGenerationController = std::make_unique<SimpleProceduralGenerationController>(64, glm::ivec2{-1, 4});
        proceduralGenerationProvider = std::make_unique<SerializedGenerationProvider>(
            std::filesystem::path("Worlds") / "Test6",
            std::make_unique<EmptyProceduralGenerationProvider>()
        );
    }

    m_camera = std::make_unique<GameCamera>(engine, Camera::ControlScheme::Developer);
    VoxelWorld::Settings voxelSettings = {
        .LoadBalanceMeshing = !Profiling::IS_PROFILING,
        .AllowFrustumCulling = true,
        .AllowBackfaceCulling = true
    };
    auto tempWorld = std::make_unique<VoxelWorld>(
        engine,
        std::make_unique<SamplingOffsets>(std::string(GetAssetsDirectory()) + "/LODSamplingOffsets.bin"),
        [this] { RecreatePipeline(); },
        std::move(proceduralGenerationProvider),
        std::move(proceduralGenerationController),
        *m_camera,
        voxelSettings
    );

    constexpr glm::vec3 CORNFLOWER_BLUE = {0.392, 0.584, 0.929};
    m_voxelRenderer = std::make_unique<VoxelRenderer>(*m_engine, *m_camera, std::move(tempWorld), CORNFLOWER_BLUE, [](VoxelTypeRegistry &voxelTypeRegistry) {
        if (WORLD_NAME == std::string("Test8")) {
            RegisterMinecraftVoxelTypes(voxelTypeRegistry);
            info("Registered {} Minecraft voxel types", voxelTypeRegistry.GetTypes().size());
        } else {
            voxelTypeRegistry.RegisterTypes(std::vector<VoxelTypeInfo>{
                {
                    1, {
                        std::string(GetAssetsDirectory()) + "/grass_top.png",
                        std::string(GetAssetsDirectory()) + "/dirt.png",
                        std::string(GetAssetsDirectory()) + "/grass_side.png"
                    },
                    SPIRE_VOXEL_LAYOUT_TOP_DIFFERENT_BOTTOM_DIFFERENT
                },
                {2, {std::string(GetAssetsDirectory()) + "/dirt.png"}, SPIRE_VOXEL_LAYOUT_ALL_SAME},
            });
        }
    });
    VoxelWorld &world = m_voxelRenderer->GetWorld();

    m_voxelRenderer->GetWorld().GetRenderer().HandleChunkEdits(m_camera->GetPosition());

    if (Profiling::IS_PROFILING) {
        VoxelSerializer::ClearAndDeserialize(world, std::filesystem::path("Worlds") / WORLD_NAME);
    } else if (!ShouldStreamLoading()) {
        VoxelSerializer::ClearAndDeserialize(world, std::filesystem::path("Worlds") / WORLD_NAME);
    }
    info("Loaded {} chunks from world file {}", world.NumLoadedChunks(), WORLD_NAME);

    //  world.LoadChunks({{-1, 0, 0}});
    //  world.LoadChunks({{0, 0, 0}});
    // CuboidVoxelEdit({0,0,0},{64,64,64},{128}).Apply(world);
    // CuboidVoxelEdit({64,0,0},{64,64,64},{2}).Apply(world);

    // BasicVoxelEdit({
    //     BasicVoxelEdit::Edit{{0, 5, 5}, 2},
    //     BasicVoxelEdit::Edit{{1, 6, 5}, 1},
    //     BasicVoxelEdit::Edit{{1, 5, 5}, 1},
    // }).Apply(world);

    // std::vector<BasicVoxelEdit::Edit> edits;
    // for (int x = 0; x < 64; x++) {
    //     for (int y = 0; y < 64; y++) {
    //         for (int z = 0; z < 64; z++) {
    //             VoxelType type = static_cast<VoxelType>(((x + y + z) & 1) ? 1 : 2);
    //          edits.push_back(BasicVoxelEdit::Edit{{x, y, z}, static_cast<VoxelType>(z % 2 == 0 ? 1 : 2)});
    //           //  edits.push_back(BasicVoxelEdit::Edit{{x, y, z}, type});
    //         }
    //     }
    // }

    //  BasicVoxelEdit(edits).Apply(world);

    // every second voxel is set to 2 (max vertices in a chunk)
    // world.UnloadAllChunks();
    // world.LoadChunks({{0, 0, 0}});
    // std::vector<BasicVoxelEdit::Edit> edits;
    // for (int x = 0; x < 64; x++) {
    //     for (int y = 0; y < 64; y++) {
    //         for (int z = 0; z < 64; z++) {
    //             if ((x + y + z) % 2 == 0) {
    //                 edits.push_back(BasicVoxelEdit::Edit{{x, y, z}, 2});
    //             }
    //         }
    //     }
    // }
    // BasicVoxelEdit(edits).Apply(world);
    // m_voxelRenderer->GetWorld().GetRenderer().HandleChunkEdits(m_camera->GetPosition());

    // BasicVoxelEdit({
    //     BasicVoxelEdit::Edit{{0, 0, 5}, 1},
    //     BasicVoxelEdit::Edit{{1, 0, 5}, 2},
    // }).Apply(world);


    //  CuboidVoxelEdit({0, 0, 0}, {64, 64, 64}, 1).Apply(world);

    //
    // BasicVoxelEdit({
    //    BasicVoxelEdit::Edit{{16, 16, 16}, 1},
    //    BasicVoxelEdit::Edit{{17, 16, 16}, 1},
    //    BasicVoxelEdit::Edit{{17, 16, 15}, 0},
    //    BasicVoxelEdit::Edit{{17, 17, 16}, 1}
    //  }).Apply(world);

    //world.LoadChunk({0, 0, 0});
    // BasicVoxelEdit({
    //     BasicVoxelEdit::Edit{{0, 0, 0}, 2},
    //     BasicVoxelEdit::Edit{{3, 0, 1}, 1},
    //     BasicVoxelEdit::Edit{{2, 0, 0}, 1},
    //     BasicVoxelEdit::Edit{{3, 0, 0}, 2},
    //     BasicVoxelEdit::Edit{{5, 0, 5}, 2},
    //     BasicVoxelEdit::Edit{{0, 0, -15}, 2},
    // }).Apply(world);

    //  CuboidVoxelEdit({0, 0, 0}, {64, 64, 64}, 1).Apply(world);
    // world.GetRenderer().HandleChunkEdits();
    //   for (VoxelType voxelType : world.GetLoadedChunk({0, 0, 0})->VoxelData) {
    //       assert(voxelType == 1);
    //   }

    // Stress test: (lots of small faces)
    // world.UnloadAllChunks();
    // constexpr glm::ivec3 SIZE = {6,4,6};
    // for (int x = -SIZE.x / 2; x <= SIZE.x / 2; x++) {
    //     for (int y = -SIZE.y / 2; y <= SIZE.y / 2; y++) {
    //         for (int z = -SIZE.z / 2; z <= SIZE.z / 2; z++) {
    //             Chunk &chunk = world.LoadChunk({x, y, z});
    //             for (int i = 0; i < SPIRE_VOXEL_CHUNK_VOLUME; i++) {
    //                 chunk.SetVoxel(i, m_engine->GetRandom().RandomInt(0,2));
    //             }
    //             world.GetRenderer().NotifyChunkEdited(chunk);
    //         }
    //     }
    // }

    // Stress test: (big world)
    // should be profiling!!
    // copies the current world n*n-1 times for a nxn region
    //
    // std::unordered_map<glm::ivec3, Chunk *> chunks;
    //
    // glm::ivec3 minChunkPos{INT32_MAX};
    // glm::ivec3 maxChunkPos{INT32_MIN};
    //
    // for (auto &[pos, chunk] : world) {
    //     chunks.emplace(pos, chunk.get());
    //
    //     minChunkPos = glm::min(minChunkPos, pos);
    //     maxChunkPos = glm::max(maxChunkPos, pos);
    // }
    //
    // if (chunks.empty())
    //     return;
    //
    // glm::ivec3 worldSize = maxChunkPos - minChunkPos + glm::ivec3(1);
    //
    // constexpr glm::ivec3 SIZE{9, 1, 9};
    //
    // for (int sx = 0; sx < SIZE.x; ++sx) {
    //     info("progress: {}/{}", sx, SIZE.x);
    //     int ox = sx - SIZE.x / 2;
    //
    //     for (int sy = 0; sy < SIZE.y; ++sy) {
    //         int oy = sy - SIZE.y / 2;
    //
    //         for (int sz = 0; sz < SIZE.z; ++sz) {
    //             int oz = sz - SIZE.z / 2;
    //
    //             if (ox == 0 && oy == 0 && oz == 0)
    //                 continue;
    //
    //             glm::ivec3 offset{ox, oy, oz};
    //             glm::ivec3 baseDst = minChunkPos + offset * worldSize;
    //
    //             for (int i = 0; i < worldSize.x; ++i) {
    //                 for (int j = 0; j < worldSize.y; ++j) {
    //                     for (int k = 0; k < worldSize.z; ++k) {
    //                         glm::ivec3 local{i, j, k};
    //                         glm::ivec3 srcPos = minChunkPos + local;
    //
    //                         auto it = chunks.find(srcPos);
    //                         if (it == chunks.end())
    //                             continue;
    //
    //                         Chunk *src = it->second;
    //
    //                         glm::ivec3 dstPos = baseDst + local;
    //                         Chunk &dst = world.LoadChunk(dstPos);
    //
    //                         dst.VoxelData = src->VoxelData;
    //                         dst.VoxelBits = src->VoxelBits;
    //
    //                         world.GetRenderer().NotifyChunkEdited(dst);
    //                     }
    //                 }
    //             }
    //         }
    //     }
    // }

    m_profiling = std::make_unique<Profiling>(*m_engine, *m_voxelRenderer, *m_camera);

    int numMeshesNotSupporing16BitIndices = 0;
    int maxVerticesInMesh = 0;
    int totalVertices = 0;
    for (auto &[_,chunk] : m_voxelRenderer->GetWorld()) {
        for (auto numVertices : chunk->NumVertices) {
            if (numVertices >= pow(2, 16))numMeshesNotSupporing16BitIndices++;
            if (numVertices > maxVerticesInMesh) maxVerticesInMesh = numVertices;
            totalVertices += numVertices;
        }
    }
    info("numMeshesNotSupporing16BitIndices: {}, maxVerticesInMesh {}, totalVertices: {}", numMeshesNotSupporing16BitIndices, maxVerticesInMesh, totalVertices);
}

GameApplication::~GameApplication() {
    Cleanup();
}

void GameApplication::Cleanup() {
    m_voxelRenderer.reset();
}

void GameApplication::Update() {
    m_voxelRenderer->Update();
    m_profiling->Update();

    m_camera->Update();
    RaycastUtils::Hit hit = RaycastUtils::Raycast(m_voxelRenderer->GetWorld(), m_camera->GetCamera().GetPosition(), m_camera->GetCamera().GetForward(), 10);
    if (hit) {
        Chunk *chunkOfHitVoxel = m_voxelRenderer->GetWorld().TryGetLoadedChunk(VoxelWorld::GetChunkPositionOfVoxel(hit.VoxelPosition));

        if (m_engine->GetWindow().IsKeyPressed(GLFW_KEY_L) && chunkOfHitVoxel) {
            glm::ivec3 adjacentVoxel = hit.VoxelPosition + FaceToDirection(hit.Face);
            BasicVoxelEdit(BasicVoxelEdit::Edit{.Position = adjacentVoxel, .Type = 1}).Apply(m_voxelRenderer->GetWorld());
        }

        if (m_engine->GetWindow().IsKeyPressed(GLFW_KEY_K) && chunkOfHitVoxel) {
            BasicVoxelEdit(BasicVoxelEdit::Edit{.Position = hit.VoxelPosition, .Type = 0}).Apply(m_voxelRenderer->GetWorld());
        }
    }
}

void GameApplication::Render() {
    m_frame++;

    auto &rm = m_engine->GetRenderingManager();

    m_swapchainImageIndex = rm.GetQueue().AcquireNextImage();
    if (m_swapchainImageIndex == rm.GetQueue().INVALID_IMAGE_INDEX) return;

    VkCommandBuffer commandBuffer = m_voxelRenderer->Render(m_swapchainImageIndex);
    if (commandBuffer == VK_NULL_HANDLE) return;

    RenderUi();

    std::array commandBuffersToSubmit = {
        rm.GetRenderer().GetBeginRenderingCommandBuffer(m_swapchainImageIndex),
        commandBuffer,
        rm.GetImGuiRenderer().PrepareCommandBuffer(m_swapchainImageIndex),
        rm.GetRenderer().GetEndRenderingCommandBuffer(m_swapchainImageIndex)
    };
    rm.GetQueue().SubmitRenderCommands(commandBuffersToSubmit.size(), commandBuffersToSubmit.data());

    rm.GetQueue().Present(m_swapchainImageIndex);
}

void GameApplication::RenderUi() const {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();

    ImGui::Begin(GetApplicationName(), nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS) (frame %d (swapchain image %d))", m_engine->GetDeltaTime() * 1000, 1.0f / m_engine->GetDeltaTime(), m_frame,
                m_swapchainImageIndex);

    ImGui::Text("Initial World: %s", WORLD_NAME);

    glm::u32 numChunksGeneratedThisFrame = m_voxelRenderer->GetWorld().GetProceduralGenerationManager().NumChunksGeneratedThisFrame();
    if (numChunksGeneratedThisFrame) {
        ImGui::TextColored({1, 0, 0, 1}, "Generated %d chunks this frame", numChunksGeneratedThisFrame);
    }

    const CameraInfo &cameraInfo = m_camera->GetCameraInfo();
    glm::vec3 forward = m_camera->GetCamera().GetForward();
    std::string targetedVoxelStr = "None";
    RaycastUtils::Hit hit = RaycastUtils::Raycast(m_voxelRenderer->GetWorld(), m_camera->GetCamera().GetPosition(), forward, 10);
    if (hit) {
        targetedVoxelStr = std::format(
            "({}, {}, {}), Voxel Type: {}, Targeted Face: {}",
            hit.VoxelPosition.x,
            hit.VoxelPosition.y,
            hit.VoxelPosition.z,
            m_voxelRenderer->GetWorld().GetVoxelAt(hit.VoxelPosition),
            FaceToString(hit.Face)
        );
    } else if (hit.TerminatedInLODChunk) {
        targetedVoxelStr = "Cannot raycast in LOD chunk";
    }
    ImGui::Text("Targeted Voxel: %s", targetedVoxelStr.c_str());

    if (abs(forward.x) > abs(forward.y) && abs(forward.x) > abs(forward.z)) ImGui::Text("Facing %s X", forward.x > 0 ? "Positive" : "Negative");
    else if (abs(forward.y) > abs(forward.x) && abs(forward.y) > abs(forward.z)) ImGui::Text("Facing %s Y", forward.y > 0 ? "Positive" : "Negative");
    else ImGui::Text("Facing %s Z", forward.z > 0 ? "Positive" : "Negative");

    if (ImGui::Button("Remesh All Chunks")) {
        MergedVoxelEdit edit;
        for (auto &[_,chunk] : m_voxelRenderer->GetWorld()) {
            VoxelType newVoxelType = chunk->VoxelData[0] == 0 ? 1 : 0;
            edit.With(BasicVoxelEdit{{chunk->ChunkPosition * SPIRE_VOXEL_CHUNK_SIZE}, newVoxelType});
        }
        edit.Apply(m_voxelRenderer->GetWorld());
    }

    glm::vec3 cameraPos = m_camera->GetCamera().GetPosition();
    glm::ivec3 cameraPosInt = cameraPos;
    glm::ivec3 chunkPos = {glm::floor(cameraPos.x / SPIRE_VOXEL_CHUNK_SIZE), glm::floor(cameraPos.y / SPIRE_VOXEL_CHUNK_SIZE), glm::floor(cameraPos.z / SPIRE_VOXEL_CHUNK_SIZE)};
    ImGui::Text("Chunk Position %d, %d, %d (Voxel Position %d, %d, %d)", chunkPos.x, chunkPos.y, chunkPos.z, cameraPosInt.x, cameraPosInt.y, cameraPosInt.z);

    ImGui::Text("Chunks Loaded: %d / %d (%d MB RAM / %d MB VRAM)", m_voxelRenderer->GetWorld().NumLoadedChunks(), VoxelWorldRenderer::MAXIMUM_LOADED_CHUNKS,
                static_cast<glm::u64>(std::ceil(static_cast<double>(m_voxelRenderer->GetWorld().CalculateCPUMemoryUsageForChunks()) / 1024.0 / 1024.0)),
                static_cast<glm::u64>(std::ceil(static_cast<double>(m_voxelRenderer->GetWorld().CalculateGPUMemoryUsageForChunks()) / 1024.0 / 1024.0))
    );

    glm::u64 totalRenderedVoxelFaces = 0;
    for (auto &[chunkPos,chunk] : m_voxelRenderer->GetWorld()) {
        totalRenderedVoxelFaces += chunk->TotalRenderedVoxelFaces;
    }
    ImGui::Text("Total rendered voxel faces: %d", totalRenderedVoxelFaces);

    if (m_voxelRenderer->GetWorld().GetSettings().AllowFrustumCulling) {
        int nonEmpty = m_voxelRenderer->GetWorld().GetRenderer().GetNumNonEmptyChunks();
        ImGui::Text("Frustum culled %d of %d non-empty chunks (%.1f%%)",
                    m_voxelRenderer->GetWorld().GetRenderer().GetNumChunksOutsideFrustum(),
                    nonEmpty,
                    nonEmpty == 0 ? 0.0f : 100 * m_voxelRenderer->GetWorld().GetRenderer().GetNumChunksOutsideFrustum() / static_cast<float>(nonEmpty)
        );
    } else {
        ImGui::TextColored(ImVec4{1, 0, 0, 1}, "Frustum culling is disabled!");
    }

    if (m_voxelRenderer->GetWorld().GetSettings().AllowBackfaceCulling) {
        int total = m_voxelRenderer->GetWorld().GetRenderer().GetNumBackfaceCulledFaces() + m_voxelRenderer->GetWorld().GetRenderer().GetNumNonBackfaceCulledFaces();
        ImGui::Text(
            "Backface culling culled %d of %d faces (%.1f%%) (excluding already culled chunks)",
            m_voxelRenderer->GetWorld().GetRenderer().GetNumBackfaceCulledFaces(),
            total,
            total == 0 ? 0.0f : 100 * m_voxelRenderer->GetWorld().GetRenderer().GetNumBackfaceCulledFaces() / static_cast<float>(total)
        );
    } else {
        ImGui::TextColored(ImVec4{1, 0, 0, 1}, "Backface culling is disabled!");
    }

    m_profiling->RenderUI();

    if (ImGui::CollapsingHeader("Camera")) {
        glm::vec3 cameraForward = glm::normalize(m_camera->GetCamera().GetForward());

        const char *dir;
        if (std::abs(cameraForward.x) > std::abs(cameraForward.y) && std::abs(cameraForward.x) > std::abs(cameraForward.z)) {
            dir = (cameraForward.x > 0) ? "PosX" : "NegX";
        } else if (std::abs(cameraForward.y) > std::abs(cameraForward.x) && std::abs(cameraForward.y) > std::abs(cameraForward.z)) {
            dir = (cameraForward.y > 0) ? "PosY" : "NegY";
        } else {
            dir = (cameraForward.z > 0) ? "PosZ" : "NegZ";
        }

        ImGui::Text("Facing: %s (%f, %f, %f) (Enum Value: %d)", dir, cameraForward.x, cameraForward.y, cameraForward.z, DirectionToFace(cameraForward));

        ImGui::Text("Position %f, %f, %f", cameraPos.x, cameraPos.y, cameraPos.z);

        ImGui::SliderFloat("Camera Speed: ", &m_camera->Speed, 1, 25);
        ImGui::SliderFloat("Camera Scale: ", &m_camera->Scale, 0.01, 1);

        static glm::vec3 destination = {0, 0, 0};
        ImGui::InputFloat3("Coordinates: ", &destination.x);
        if (ImGui::Button("Teleport")) {
            m_camera->GetCamera().SetPosition(destination);
        }
    }

    static int newLod = 2;
    static float chanceToLOD = 1;
    static bool canLOD = true;

    if (canLOD && ImGui::CollapsingHeader("LOD")) {
        ImGui::InputInt("New LOD Level", &newLod);
        ImGui::SliderFloat("Chance to LOD a chunk: ", &chanceToLOD, 0, 1);
        chanceToLOD = std::clamp(chanceToLOD, 0.0f, 1.0f);

        if (ImGui::Button("Generate New LOD")) {
            Timer timer;
            auto &world = m_voxelRenderer->GetWorld();
            std::vector<Chunk *> chunks;
            glm::u32 originalNumChunks = world.NumLoadedChunks();
            for (auto &[chunkCoords,chunk] : world) {
                bool shouldLOD = m_engine->GetRandom().RandomFloat() < chanceToLOD;
                if (chunkCoords.x % newLod == 0 && (chunkCoords.y + 1 /*Test5 starts at chunk y = -1*/) % newLod == 0 && chunkCoords.z % newLod == 0 && shouldLOD) {
                    chunks.push_back(chunk.get());
                }
            }
            for (Chunk *chunk : chunks) {
                world.GetLODManager().IncreaseLODTo(*chunk, newLod);
            }
            glm::u32 numConverted = chunks.size() + (originalNumChunks - world.NumLoadedChunks());
            info("{} of {} chunks converted to new LOD {} (requested {}%) in {} ms.", numConverted, originalNumChunks, newLod, static_cast<int>(chanceToLOD * 100),
                 timer.MillisSinceStart());
            canLOD = false;
        }
    }

    ImGui::End();

    ImGui::Render();
}

bool GameApplication::ShouldClose() const {
    return m_engine->GetWindow().ShouldClose();
}

const char *GameApplication::GetApplicationName() const {
    return "MyApp";
}

void GameApplication::OnWindowResize() const {
    m_voxelRenderer->OnWindowResize();
}

void GameApplication::RecreatePipeline() {
    m_voxelRenderer->RecreatePipeline();
}
