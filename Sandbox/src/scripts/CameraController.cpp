#include "CameraController.hpp"

#include <algorithm>
#include <imgui.h>

void CameraController::onUpdate() {
    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) || ImGui::IsAnyItemActive()) {
        return;
    }

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

    static const float minDist = 0.5;
    static const float maxDist = 10;
    auto &transform = getComponent<GameEngine::TransformComponent>();
    transform.localPosition.z = std::clamp(transform.localPosition.z * (1 + GameEngine::Input::wheelDeltaY() / 750.0f), minDist, maxDist);
}

void CameraController::initFromJSON(const nlohmann::json &scriptJSON) {

}