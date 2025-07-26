#pragma once

class Application {
public:
    virtual ~Application() = default;

public:
    virtual void Start() = 0;

    virtual void Update() = 0;

    virtual void Render() = 0;

    virtual bool ShouldClose() const = 0;

    virtual std::string GetApplicationName() const = 0;
};
