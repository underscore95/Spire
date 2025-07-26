#include "GameApplication.h"

void GameApplication::Start() {
    m_window = Window::Create();
}

void GameApplication::Update() {

}

void GameApplication::Render() {
}

bool GameApplication::ShouldClose() {
    return m_window->ShouldClose();
}
