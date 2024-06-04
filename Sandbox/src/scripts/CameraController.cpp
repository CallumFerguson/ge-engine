#include "CameraController.hpp"

void CameraController::onUpdate() {
    auto &transform = getComponent<GameEngine::TransformComponent>();

    if (GameEngine::Input::getKey(GameEngine::KeyCode::A)) {
        transform.localPosition[0] -= GameEngine::Time::deltaTime();
    }
    if (GameEngine::Input::getKey(GameEngine::KeyCode::D)) {
        transform.localPosition[0] += GameEngine::Time::deltaTime();
    }
    if (GameEngine::Input::getKey(GameEngine::KeyCode::W)) {
        transform.localPosition[2] -= GameEngine::Time::deltaTime();
    }
    if (GameEngine::Input::getKey(GameEngine::KeyCode::S)) {
        transform.localPosition[2] += GameEngine::Time::deltaTime();
    }
}
