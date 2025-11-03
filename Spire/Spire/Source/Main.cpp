#include <memory>
#include "Engine/Core/Engine.h"
#include "Game/GameApplication.h"

int main() {
    Spire::Engine engine(std::make_unique<GameApplication>());
}
