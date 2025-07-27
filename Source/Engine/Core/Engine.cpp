#include "Engine.h"

#include <memory>

#include "Application.h"
#include "Engine/Rendering/RenderingManager.h"
#include "Engine/Window/Window.h"

Engine::Engine(std::unique_ptr<Application> app)
    : m_application(std::move(app)),
      m_initialized(false) {
    // Window
    if (!Window::Init()) return;

    // RenderingManager
    m_renderingManager = std::make_unique<RenderingManager>(
        m_application->GetApplicationName()
    );
    if (!m_renderingManager->IsValid()) return;

    m_initialized = true;
    Start();
}

Engine::~Engine() {
    m_application.reset();

    Window::Shutdown();

    m_renderingManager.reset();
}

void Engine::Start() const {
    m_application->Start();

    while (!m_application->ShouldClose()) {
        Update();
        Render();
    }
}

void Engine::Update() const {
    Window::Update();

    m_application->Update();
}

void Engine::Render() const {
    m_application->Render();
}
