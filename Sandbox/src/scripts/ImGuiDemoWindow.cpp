#include "ImGuiDemoWindow.hpp"

#include <imgui.h>

void ImGuiDemoWindow::onImGui() {
    if (m_showDemoWindow) {
        ImGui::ShowDemoWindow(&m_showDemoWindow);
    }
}

std::string &ImGuiDemoWindow::imGuiName() {
    static std::string s_name = "ImGuiDemoWindow";
    return s_name;
}
