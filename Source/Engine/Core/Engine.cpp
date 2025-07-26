#include "Engine.h"

#include "Application.h"
#include "Engine/Window/Window.h"

Engine::Engine(std::unique_ptr<Application> app)
    : m_application(std::move(app)),
      m_initialized(false) {
    if (!Window::Init()) return;

    m_initialized = true;
    Start();
}

Engine::~Engine() {
    m_application.reset();

    Window::Shutdown();
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
