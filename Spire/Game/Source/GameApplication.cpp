#include "GameApplication.h"
#include "../../Libs/glfw/include/GLFW/glfw3.h"
#include "GameCamera.h"
#include "Utils/RaycastUtils.h"

using namespace Spire;
using namespace SpireVoxel;

GameApplication::GameApplication() {
}

void GameApplication::Start(Engine &engine) {
    m_engine = &engine;

    m_camera = std::make_unique<GameCamera>(engine);
    m_voxelRenderer = std::make_unique<VoxelRenderer>(*m_engine, *m_camera);

    m_voxelRenderer->GetWorld().GetRenderer().HandleChunkEdits();
}

GameApplication::~GameApplication() {
    Cleanup();
}

void GameApplication::Cleanup() {
    m_voxelRenderer.reset();
}

void GameApplication::Update() {
    m_voxelRenderer->Update();
    m_camera->Update();
    RaycastUtils::Hit hit = RaycastUtils::Raycast(m_voxelRenderer->GetWorld(), m_camera->GetCamera().GetPosition(), m_camera->GetCamera().GetForward(), 10);
    if (hit) {
        Chunk *chunkOfHitVoxel = m_voxelRenderer->GetWorld().GetLoadedChunk(VoxelWorld::GetChunkPositionOfVoxel(hit.VoxelPosition)).get();

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
    ImGuiIO &io = ImGui::GetIO();

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();

    ImGui::Begin(GetApplicationName(), nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS) (frame %d (swapchain image %d))", m_engine->GetDeltaTime() * 1000, 1.0f / m_engine->GetDeltaTime(), m_frame,
                m_swapchainImageIndex);

    const CameraInfo &cameraInfo = m_camera->GetCameraInfo();
    std::string targetedVoxelStr = "None";
    RaycastUtils::Hit hit = RaycastUtils::Raycast(m_voxelRenderer->GetWorld(), m_camera->GetCamera().GetPosition(), m_camera->GetCamera().GetForward(), 10);
    if (hit) {
        targetedVoxelStr = std::format(
            "({}, {}, {}), Voxel Type: {}, Targeted Face: {}",
            hit.VoxelPosition.x,
            hit.VoxelPosition.y,
            hit.VoxelPosition.z,
            m_voxelRenderer->GetWorld().GetVoxelAt(hit.VoxelPosition),
            FaceToString(hit.Face)
        );
    }
    ImGui::Text("Targeted Voxel: %s", targetedVoxelStr.c_str());

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

    glm::vec3 cameraPos = m_camera->GetCamera().GetPosition();
    ImGui::Text("Position %f, %f, %f", cameraPos.x, cameraPos.y, cameraPos.z);

    glm::vec3 chunkPos = {glm::floor(cameraPos.x / SPIRE_VOXEL_CHUNK_SIZE), glm::floor(cameraPos.y / SPIRE_VOXEL_CHUNK_SIZE), glm::floor(cameraPos.z / SPIRE_VOXEL_CHUNK_SIZE)};
    ImGui::Text("Chunk Position %f, %f, %f", chunkPos.x, chunkPos.y, chunkPos.z);

    ImGui::Text("Chunks Loaded: %d (%d MB VRAM)", m_voxelRenderer->GetWorld().NumLoadedChunks(),
                static_cast<glm::u64>(std::ceil(static_cast<double>(m_voxelRenderer->GetWorld().CalculateGPUMemoryUsageForChunks()) / 1024.0 / 1024.0)));

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
