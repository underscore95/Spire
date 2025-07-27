#include "Engine.h"

#include <memory>
#include <spdlog/spdlog.h>

#include "Application.h"
#include "Engine/Rendering/RenderingManager.h"
#include "Engine/Window/Window.h"

Engine::Engine(std::unique_ptr<Application> app)
    : m_application(std::move(app)),
      m_initialized(false) {
    spdlog::info("Initializing engine...");

    // Window
    if (!Window::Init()) return;
    m_window = Window::Create();

    // RenderingManager
    m_renderingManager = std::make_unique<RenderingManager>(
        m_application->GetApplicationName(),
        GetWindow()
    );
    if (!m_renderingManager->IsValid()) {
        spdlog::error("Failed to initialize rendering manager");
        return;
    }

    m_initialized = true;
    Start();

    spdlog::info("Initialized engine!\n");
}

Engine::~Engine() {
    spdlog::info("Shutting down engine...");

    m_application.reset();

    Window::Shutdown();

    m_renderingManager.reset();

    spdlog::info("Shut down engine!");
}

const Window &Engine::GetWindow() const {
    return *m_window;
}

const RenderingManager &Engine::GetRenderingManager() const {
    return *m_renderingManager;
}

void Engine::Start() {
    m_application->Start(*this);

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
