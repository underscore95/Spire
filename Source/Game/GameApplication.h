#pragma once

#include "Engine/EngineIncludes.h"

class GameApplication : public Application {
public:
    void Start() override;

    void Update() override;

    void Render() override;

    bool ShouldClose() const override;

    std::string GetApplicationName() const override;

private:
    std::unique_ptr<Window> m_window;
};
