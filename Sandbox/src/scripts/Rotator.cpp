#include "Rotator.hpp"

#include <glm/glm.hpp>
#include <imgui.h>

void Rotator::onUpdate() {
    auto &transform = getComponent<GameEngine::TransformComponent>();

    if (GameEngine::Input::getKey(GameEngine::KeyCode::G)) {
        transform.localRotation = glm::rotate(transform.localRotation, glm::radians(GameEngine::Time::deltaTime() * speed), glm::vec3(0.0f, 0.0f, 1.0f));
    }
}

std::string &Rotator::imGuiName() {
    static std::string s_name = "Rotator";
    return s_name;
}

void Rotator::onImGuiInspector() {
    ImGui::DragFloat("speed", &speed, 1.0f);
}
