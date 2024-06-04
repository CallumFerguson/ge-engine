#include "Rotator.hpp"

void Rotator::onUpdate() {
    auto &transform = getComponent<GameEngine::TransformComponent>();

    if (GameEngine::Input::getKey(GameEngine::KeyCode::G)) {
        transform.position[1] += GameEngine::Time::deltaTime();
    }
}
