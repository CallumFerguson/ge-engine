#include "TrackFramerate.hpp"

#include <cmath>
#include <imgui.h>

void TrackFramerate::onImGui() {
    m_fpsRollingAverage.addSample(1.0 / GameEngine::Time::deltaTime());

    ImGui::SetNextWindowSize(ImVec2(0, 0));
    ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_Always, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Info", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
    ImGui::Text("fps: %d", static_cast<int>(std::round(m_fpsRollingAverage.average())));
    ImGui::End();
}

nlohmann::json TrackFramerate::toJSON() {
    return {};
}

void TrackFramerate::initFromJSON(const nlohmann::json &scriptJSON) {

}
