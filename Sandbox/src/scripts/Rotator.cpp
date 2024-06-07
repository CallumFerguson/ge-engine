#include "Rotator.hpp"

#include <glm/glm.hpp>
#include <imgui.h>

void Rotator::onUpdate() {
    auto &transform = getComponent<GameEngine::TransformComponent>();

    if (GameEngine::Input::getKey(GameEngine::KeyCode::G)) {
        transform.localRotation = glm::rotate(transform.localRotation, glm::radians(GameEngine::Time::deltaTime() * speed), glm::vec3(0.0f, 0.0f, 1.0f));
    }
}

void Rotator::onImGuiInspector() {
    ImGui::DragFloat("speed", &speed, 1.0f);
}

nlohmann::json Rotator::toJSON() {
    nlohmann::json result;
    result["speed"] = speed;
    return result;
}

void Rotator::initFromJSON(const nlohmann::json &scriptJSON) {
    speed = scriptJSON["speed"];
}
