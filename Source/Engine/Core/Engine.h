#pragma once

#include <memory>

class Application;

class Engine {
public:
    explicit Engine(std::unique_ptr<Application> app);
    ~Engine();

private:
    void Start() const;
    void Update() const;
    void Render() const;

private:
    std::unique_ptr<Application> m_application;
    bool m_initialized;
};
