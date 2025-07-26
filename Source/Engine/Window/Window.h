#pragma once

#include <string>
#include <memory>
#include <glm/glm.hpp>

struct GLFWwindow;
class Engine;

class Window {
    friend class Engine;
private:
    Window(const std::string &title, glm::ivec2 size);

public:
    ~Window();

public:
    static std::unique_ptr<Window> Create(const std::string &title = "My Window", glm::ivec2 size = {1280, 720});

private:
    static bool Init();

    static void Shutdown();

    static void Update();

public:
    [[nodiscard]] bool IsValid() const;

    [[nodiscard]] bool ShouldClose() const;

private:
    static bool s_initialized;

    GLFWwindow *m_windowHandle;
};
