#pragma once

#include "Engine/EngineIncludes.h"

class GameApplication : public Application {
public:
    void Start(Engine &engine) override;

    void Update() override;

    void Render() override;

    bool ShouldClose() const override;

    std::string GetApplicationName() const override;

private:
    Engine* m_engine;
};
