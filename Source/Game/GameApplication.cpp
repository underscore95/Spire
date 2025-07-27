#include "GameApplication.h"

void GameApplication::Start(Engine &engine) {
    m_engine = &engine;
}

void GameApplication::Update() {
}

void GameApplication::Render() {
}

bool GameApplication::ShouldClose() const {
    return m_engine->GetWindow().ShouldClose();
}

std::string GameApplication::GetApplicationName() const {
    return "MyApp";
}
