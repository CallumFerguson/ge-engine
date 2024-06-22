#include "CameraController.hpp"

void CameraController::onUpdate() {
    auto &transform = getComponent<GameEngine::TransformComponent>();

    auto pivotX = getEntity().getParent();
    auto pivotY = pivotX.getParent();

    if (GameEngine::Input::getMouseButton(GameEngine::Input::MouseButton::left)) {
        float speed = 0.25f;
        pivotY.getComponent<GameEngine::TransformComponent>().localRotation *= glm::angleAxis(glm::radians(GameEngine::Input::mouseDeltaX() * speed), glm::vec3(0.0f, 1.0f, 0.0f));

        auto &pivotXTransform = pivotX.getComponent<GameEngine::TransformComponent>();
        auto pivotXEulerAngles = glm::degrees(glm::eulerAngles(pivotXTransform.localRotation));
        pivotXEulerAngles.x += GameEngine::Input::mouseDeltaY() * speed;
        if (pivotXEulerAngles.x > 90.0f) {
            pivotXEulerAngles.x = 90.0f;
        } else if (pivotXEulerAngles.x < -90.0f) {
            pivotXEulerAngles.x = -90.0f;
        }
        pivotXTransform.localRotation = glm::quat(glm::radians(pivotXEulerAngles));
    }
}

void CameraController::initFromJSON(const nlohmann::json &scriptJSON) {

}