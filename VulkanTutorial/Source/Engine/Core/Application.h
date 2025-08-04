#pragma once

class Engine;

class Application {
public:
    virtual ~Application() = default;

public:
    virtual void Start(Engine& engine) = 0;

    virtual void Update() = 0;

    virtual void Render() = 0;

    virtual bool ShouldClose() const = 0;

    virtual const char* GetApplicationName() const = 0;
};
