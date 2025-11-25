#pragma once

#include "pch.h"

struct GLFWwindow;

// Adapted from: https://github.com/underscore95/Chimp/blob/main/Chimp/ChimpFramework/Source/api/window/IWindow.h
namespace Spire
{
    class Engine;

    class Window
    {
        friend class Engine;

    private:
        Window(const std::string& title, glm::ivec2 size, Engine& engine);

    public:
        ~Window();

    public:
        [[nodiscard]] static std::unique_ptr<Window> Create(
            Engine& engine,
            const std::string& title = "My Window",
            glm::ivec2 size = {1280, 720}
        );

        [[nodiscard]] bool IsKeyHeld(int key) const;

        [[nodiscard]] bool IsKeyPressed(int key) const;

        [[nodiscard]] bool IsMouseButtonHeld(int button) const;

        [[nodiscard]] bool IsMouseButtonPressed(int button) const;

        [[nodiscard]] glm::vec2 GetMousePos() const;

        [[nodiscard]] glm::vec2 GetMouseDelta() const;

        [[nodiscard]] glm::vec2 GetScrollDelta() const;

        [[nodiscard]] float GetAspectRatio() const;

        void KeyCallback(int key, int scanCode, int action, int mods);

        void MouseCallback(double xPos, double yPos);

        void MouseButtonCallback(int button, int action, int mods);

        void ScrollCallback(double xOffset, double yOffset);

        // Recheck the dimensions early, usually we only recheck the dimensions at the beginning of each frame
        // this doesn't call engine::OnWindowResize();
        void UpdateDimensions();

    private:
        static bool Init();

        static void Shutdown();

        void Update();

        void LateUpdate();

    public:
        [[nodiscard]] bool IsValid() const;

        [[nodiscard]] bool ShouldClose() const;

        [[nodiscard]] GLFWwindow* GLFWWindow() const;

        [[nodiscard]] glm::uvec2 GetDimensions() const;

    private:
        static bool s_initialized;

        GLFWwindow* m_windowHandle;
        Engine& m_engine;
        std::unordered_set<int> m_pressedKeys;
        std::unordered_set<int> m_heldKeys;
        std::unordered_set<int> m_pressedMouseButtons;
        std::unordered_set<int> m_heldMouseButtons;
        glm::vec2 m_mousePos;
        glm::vec2 m_mouseRawPos;
        glm::vec2 m_mouseDelta;
        glm::vec2 m_scrollDelta;
        glm::vec2 m_scrollDeltaTemp;
        glm::uvec2 m_windowDimensions;
        glm::uvec2 m_windowDimensionsLastUpdate;
    };
}