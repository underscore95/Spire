#include "Engine.h"

#include <memory>
#include <spdlog/spdlog.h>

#include "Application.h"
#include "Engine/Rendering/RenderingManager.h"
#include "Engine/Window/Window.h"

Engine::Engine(std::unique_ptr<Application> app)
    : m_application(std::move(app)),
      m_initialized(false),
      m_beginInitializationTimePoint(std::chrono::high_resolution_clock::now()) {
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
    const auto now = std::chrono::high_resolution_clock::now();
    spdlog::info("Initialized engine in {} ms!\n",
                 std::chrono::duration_cast<std::chrono::milliseconds>(now - m_beginInitializationTimePoint).count());
    Start();
}

Engine::~Engine() {
    const auto beginShutdown = std::chrono::high_resolution_clock::now();

    spdlog::info("");
    spdlog::info("Shutting down application...");

    m_application.reset();

    const auto afterAppShutdown = std::chrono::high_resolution_clock::now();

    spdlog::info("");
    spdlog::info("Shutting down engine...");

    Window::Shutdown();

    m_renderingManager.reset();

    const auto afterEngineShutdown = std::chrono::high_resolution_clock::now();

    spdlog::info("Shut down engine!");

    const auto appShutdownTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        afterAppShutdown - beginShutdown).count();
    const auto engineShutdownTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        afterEngineShutdown - afterAppShutdown).count();
    const auto totalShutdownTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        afterEngineShutdown - beginShutdown).count();

    spdlog::info("Application shutdown took {} ms", appShutdownTime);
    spdlog::info("Engine shutdown took {} ms", engineShutdownTime);
    spdlog::info("Total shutdown time: {} ms\n", totalShutdownTime);
}

const Window &Engine::GetWindow() const {
    return *m_window;
}

RenderingManager &Engine::GetRenderingManager() const {
    return *m_renderingManager;
}

// ReSharper disable once CppDFAConstantFunctionResult
float Engine::GetDeltaTime() const {
    return m_deltaTime;
}

void Engine::Start() {
    spdlog::info("Initializing application...");
    const auto beginApplicationInit = std::chrono::high_resolution_clock::now();
    m_application->Start(*this);
    const auto now = std::chrono::high_resolution_clock::now();
    spdlog::info("Initialized application in {} ms! (engine+app initialized in: {} ms)\n",
                 std::chrono::duration_cast<std::chrono::milliseconds>(now - beginApplicationInit).count(),
                 std::chrono::duration_cast<std::chrono::milliseconds>(now - m_beginInitializationTimePoint).count());

    while (!m_application->ShouldClose()) {
        m_deltaTimeTimer.Start();
        Update();
        Render();
        m_deltaTime = m_deltaTimeTimer.SecondsSinceStart();
    }
}

void Engine::Update() const {
    Window::Update();

    m_application->Update();
}

void Engine::Render() const {
    m_application->Render();
}
