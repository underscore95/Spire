#pragma once

#include <memory>
#include <string>
class RenderingManager;
class Window;

class Application;

class Engine {
public:
    explicit Engine(std::unique_ptr<Application> app);

    ~Engine();

public:
    [[nodiscard]] const Window &GetWindow() const;

    [[nodiscard]] const RenderingManager& GetRenderingManager() const;

private:
    void Start();

    void Update() const;

    void Render() const;

public:
    static inline const std::string s_engineName = "VulkanEngine";

private:
    std::unique_ptr<Application> m_application;
    bool m_initialized;

    std::unique_ptr<RenderingManager> m_renderingManager;

    std::unique_ptr<Window> m_window;
};
