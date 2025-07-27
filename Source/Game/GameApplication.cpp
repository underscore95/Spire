#include "GameApplication.h"

#include "Engine/Rendering/RenderingManager.h"

void GameApplication::Start(Engine &engine) {
    m_engine = &engine;

    auto &rm = m_engine->GetRenderingManager();

    // Command buffers
    m_commandBuffers.resize(rm.GetNumImages());
    rm.CreateCommandBuffers(rm.GetNumImages(), m_commandBuffers.data());
}

GameApplication::~GameApplication() {
    auto &rm = m_engine->GetRenderingManager();

    rm.FreeCommandBuffers(m_commandBuffers.size(), m_commandBuffers.data());
}

void GameApplication::Update() {
}

void GameApplication::Render() {
}

bool GameApplication::ShouldClose() const {
    return m_engine->GetWindow().ShouldClose();
}

std::string GameApplication::GetApplicationName() const {
    return "MyApp";
}
