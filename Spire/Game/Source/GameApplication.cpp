#include "GameApplication.h"

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
}

void GameApplication::Render() {
    auto &rm = m_engine->GetRenderingManager();

    glm::u32 imageIndex = rm.GetQueue().AcquireNextImage();
    if (imageIndex == rm.GetQueue().INVALID_IMAGE_INDEX) return;

    VkCommandBuffer commandBuffer = m_voxelRenderer->Render(imageIndex);
    if (commandBuffer == VK_NULL_HANDLE) return;

    RenderUi();

    std::array commandBuffersToSubmit = {
        rm.GetRenderer().GetBeginRenderingCommandBuffer(imageIndex),
        commandBuffer,
        rm.GetImGuiRenderer().PrepareCommandBuffer(imageIndex),
        rm.GetRenderer().GetEndRenderingCommandBuffer(imageIndex)
    };
    rm.GetQueue().SubmitRenderCommands(commandBuffersToSubmit.size(), commandBuffersToSubmit.data());

    rm.GetQueue().Present(imageIndex);
}

void GameApplication::RenderUi() const {
    ImGuiIO &io = ImGui::GetIO();

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();

    ImGui::Begin(GetApplicationName(), nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

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
