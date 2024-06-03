#include "CameraController.hpp"

void CameraController::onUpdate() {
    auto &transform = getComponent<GameEngine::TransformComponent>();

    if (GameEngine::Input::getKey(GameEngine::KeyCode::A)) {
        transform.position[0] -= GameEngine::Time::deltaTime();
    }
    if (GameEngine::Input::getKey(GameEngine::KeyCode::D)) {
        transform.position[0] += GameEngine::Time::deltaTime();
    }
    if (GameEngine::Input::getKey(GameEngine::KeyCode::W)) {
        transform.position[2] -= GameEngine::Time::deltaTime();
    }
    if (GameEngine::Input::getKey(GameEngine::KeyCode::S)) {
        transform.position[2] += GameEngine::Time::deltaTime();
    }
}
