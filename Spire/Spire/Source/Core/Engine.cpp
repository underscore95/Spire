#include "Engine.h"

#include "Utils/Log.h"

#include "Application.h"
#include "Rendering/RenderingManager.h"
#include "Resources/ImageLoader.h"
#include "Window/Window.h"

namespace Spire {
    const Version INVALID_VERSION{0, 0, 0};

    Engine::Engine(std::unique_ptr<Application> app)
        : m_application(std::move(app)),
          m_initialized(false),
          m_beginInitializationTimePoint(std::chrono::high_resolution_clock::now()),
          m_version(Version::FromString(SPIRE_VERSION).value_or(INVALID_VERSION)) {
        info("Initializing engine...");

        if (m_version == INVALID_VERSION) warn("Failed to parse {} as a version", SPIRE_VERSION);
        else info("Spire v{}",SPIRE_VERSION);

        // Window
        if (!Window::Init()) return;
        m_window = Window::Create(*this);

        // RenderingManager
        m_renderingManager = std::make_unique<RenderingManager>(
            *this,
            m_application->GetApplicationName(),
            *m_window,
            3 // triple buffering
        );
        if (!m_renderingManager->IsValid()) {
            error("Failed to initialize rendering manager");
            return;
        }

        m_initialized = true;
        const auto now = std::chrono::high_resolution_clock::now();
        info("Initialized engine in {} ms!\n",
             std::chrono::duration_cast<std::chrono::milliseconds>(now - m_beginInitializationTimePoint).
             count());
        Start();
    }

    Engine::~Engine() {
        const auto beginShutdown = std::chrono::high_resolution_clock::now();

        info("");
        info("Shutting down application...");

        m_application.reset();

        const auto afterAppShutdown = std::chrono::high_resolution_clock::now();

        info("");
        info("Shutting down engine...");

        Window::Shutdown();

        m_renderingManager.reset();

        if (ImageLoader::GetNumLoadedImages() != 0) {
            error("There was {} images still loaded when the engine shut down!",
                  ImageLoader::GetNumLoadedImages());
        }

        const auto afterEngineShutdown = std::chrono::high_resolution_clock::now();

        info("Shut down engine!");

        const auto appShutdownTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            afterAppShutdown - beginShutdown).count();
        const auto engineShutdownTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            afterEngineShutdown - afterAppShutdown).count();
        const auto totalShutdownTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            afterEngineShutdown - beginShutdown).count();

        info("Application shutdown took {} ms", appShutdownTime);
        info("Engine shutdown took {} ms", engineShutdownTime);
        info("Total shutdown time: {} ms\n", totalShutdownTime);
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
        info("Initializing application...");
        const auto beginApplicationInit = std::chrono::high_resolution_clock::now();
        m_application->Start(*this);
        const auto now = std::chrono::high_resolution_clock::now();
        info("Initialized application in {} ms! (engine+app initialized in: {} ms)\n",
             std::chrono::duration_cast<std::chrono::milliseconds>(now - beginApplicationInit).count(),
             std::chrono::duration_cast<std::chrono::milliseconds>(now - m_beginInitializationTimePoint).
             count());

        while (!m_application->ShouldClose()) {
            m_deltaTimeTimer.Restart();
            Update();
            Render();
            m_deltaTime = m_deltaTimeTimer.SecondsSinceStart();
        }
    }

    void Engine::Update() const {
        m_window->Update();

        m_application->Update();
    }

    void Engine::Render() const {
        if (!m_isMinimized) {
            m_application->Render();
        }
    }

    void Engine::OnWindowResize() {
        m_window->UpdateDimensions();
        m_isMinimized = m_window->GetDimensions().x == 0 || m_window->GetDimensions().y == 0;

        if (!m_isMinimized) {
            m_renderingManager->OnWindowResize();
            m_application->OnWindowResize();
        }
    }
}
