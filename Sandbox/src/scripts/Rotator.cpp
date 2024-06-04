#include "Rotator.hpp"

#include <glm/glm.hpp>

void Rotator::onUpdate() {
    auto &transform = getComponent<GameEngine::TransformComponent>();

    if (GameEngine::Input::getKey(GameEngine::KeyCode::G)) {
        transform.localRotation = glm::rotate(transform.localRotation, glm::radians(GameEngine::Time::deltaTime() * speed), glm::vec3(0.0f, 0.0f, 1.0f));
    }
}
