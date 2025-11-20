#include "GameApplication.h"
#include "../../Libs/glfw/include/GLFW/glfw3.h"
#include "Rendering/GameCamera.h"

using namespace Spire;

GameApplication::GameApplication() {
    // don't do anything here!
}

void GameApplication::Start(Engine &engine) {
    m_engine = &engine;

    m_voxelRenderer = std::make_unique<SpireVoxel::VoxelRenderer>(*m_engine);
}

GameApplication::~GameApplication() {
    Cleanup();
}

void GameApplication::Cleanup() {
    m_voxelRenderer.reset();
}

void GameApplication::Update() {
    m_voxelRenderer->Update();

    std::optional emptyVoxel = m_voxelRenderer->GetCamera().GetTargetedAdjacentEmptyVoxelPosition();
    if (m_engine->GetWindow().IsKeyPressed(GLFW_KEY_L) && emptyVoxel.has_value()) {
        m_voxelRenderer->GetWorld().TrySetVoxelAt(*emptyVoxel, 1);
    }

    std::optional targetVoxel = m_voxelRenderer->GetCamera().GetTargetedVoxelPosition();
    if (m_engine->GetWindow().IsKeyPressed(GLFW_KEY_K) && targetVoxel.has_value()) {
        m_voxelRenderer->GetWorld().TrySetVoxelAt(*targetVoxel, 0);
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

    const SpireVoxel::CameraInfo &cameraInfo = m_voxelRenderer->GetCamera().GetCameraInfo();
    std::string targetedVoxelStr = "None";
    if (cameraInfo.IsTargetingVoxel) {
        targetedVoxelStr = std::format("({}, {}, {}) (Voxel Type: {})",
                                       cameraInfo.TargetedVoxelX,
                                       cameraInfo.TargetedVoxelY,
                                       cameraInfo.TargetedVoxelZ,
                                       m_voxelRenderer->GetWorld().GetVoxelAt(glm::ivec3{
                                           cameraInfo.TargetedVoxelX, cameraInfo.TargetedVoxelY, cameraInfo.TargetedVoxelZ
                                       }));
    }
    ImGui::Text("Targeted Voxel: %s", targetedVoxelStr.c_str());

    glm::vec3 cameraForward = glm::normalize(m_voxelRenderer->GetCamera().GetCamera().GetForward());

    const char *dir;
    if (std::abs(cameraForward.x) > std::abs(cameraForward.y) && std::abs(cameraForward.x) > std::abs(cameraForward.z)) {
        dir = (cameraForward.x > 0) ? "PosX" : "NegX";
    } else if (std::abs(cameraForward.y) > std::abs(cameraForward.x) && std::abs(cameraForward.y) > std::abs(cameraForward.z)) {
        dir = (cameraForward.y > 0) ? "PosY" : "NegY";
    } else {
        dir = (cameraForward.z > 0) ? "PosZ" : "NegZ";
    }

    ImGui::Text("Facing: %s (%f, %f, %f)", dir, cameraForward.x, cameraForward.y, cameraForward.z);

    glm::vec3 cameraPos = m_voxelRenderer->GetCamera().GetCamera().GetPosition();
    ImGui::Text("Position %f, %f, %f", cameraPos.x, cameraPos.y, cameraPos.z);

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
