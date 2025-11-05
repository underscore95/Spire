#pragma once

#include "pch.h"
#include "Utils/Timer.h"
#include "Utils/Version.h"

namespace Spire
{
    class RenderingManager;
    class Window;
    class Application;

    class Engine
    {
    public:
        explicit Engine(std::unique_ptr<Application> app);

        ~Engine();

    public:
        [[nodiscard]] const Window& GetWindow() const;

        [[nodiscard]] RenderingManager& GetRenderingManager() const;

        [[nodiscard]] float GetDeltaTime() const;

        void OnWindowResize();

    private:
        void Start();

        void Update() const;

        void Render() const;

    public:
        static inline const std::string s_engineName = "VulkanEngine";

    private:
        std::unique_ptr<Application> m_application;
        bool m_initialized;
        const std::chrono::time_point<std::chrono::high_resolution_clock> m_beginInitializationTimePoint;

        std::unique_ptr<RenderingManager> m_renderingManager;

        std::unique_ptr<Window> m_window;

        Timer m_deltaTimeTimer;
        float m_deltaTime = 1.0f / 1000.0f;

        bool m_isMinimized = false;

        Version m_version;
    };
}
