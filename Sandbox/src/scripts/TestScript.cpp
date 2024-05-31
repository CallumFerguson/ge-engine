#include "TestScript.hpp"

#include <iostream>
#include <imgui.h>
#include "GameEngine.hpp"

void TestScript::onStart() {
    auto &transform = getComponent<GameEngine::TransformComponent>();
    std::cout << "start: " << transform.position[0] << std::endl;
}

void TestScript::onUpdate() {
    auto &transform = getComponent<GameEngine::TransformComponent>();
    transform.position[0] += 1;
    std::cout << "update: " << transform.position[0] << ", name: "
              << getComponent<GameEngine::NameComponent>().name << std::endl;
}

void TestScript::onImGui() {
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 10.0f, 10.0f), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    ImGui::Begin("Controls", nullptr,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    if (ImGui::Button("Test")) {
        std::cout << "test button pressed" << std::endl;
    }
    ImGui::End();
}
