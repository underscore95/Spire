#pragma once

#include <memory>
#include <string>
class RenderingManager;

class Application;

class Engine {
public:
    explicit Engine(std::unique_ptr<Application> app);

    ~Engine();

private:
    void Start() const;

    void Update() const;

    void Render() const;

public:
    static inline const std::string s_engineName = "VulkanEngine";

private:
    std::unique_ptr<Application> m_application;
    bool m_initialized;

    std::unique_ptr<RenderingManager> m_renderingManager;
};
