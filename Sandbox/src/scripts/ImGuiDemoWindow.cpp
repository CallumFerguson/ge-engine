#include "ImGuiDemoWindow.hpp"

#include <imgui.h>

void ImGuiDemoWindow::onImGui() {
    if (m_showDemoWindow) {
        ImGui::ShowDemoWindow(&m_showDemoWindow);
    }
}

void ImGuiDemoWindow::initFromJSON(const nlohmann::json &scriptJSON) {

}
