#include "Engine/Core/Engine.h"
#include "Game/GameApplication.h"

int main() {
    Engine engine(std::make_unique<GameApplication>());
}
