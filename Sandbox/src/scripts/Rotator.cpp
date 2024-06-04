#include "Rotator.hpp"

#include <glm/glm.hpp>

void Rotator::onUpdate() {
    auto &transform = getComponent<GameEngine::TransformComponent>();

    if (GameEngine::Input::getKey(GameEngine::KeyCode::G)) {
        transform.position[1] += GameEngine::Time::deltaTime() * speed;
//        transform.rotation = glm::rotate(transform.rotation, glm::radians(GameEngine::Time::deltaTime() * 10.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    }
}
