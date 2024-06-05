#pragma once

#include "GameEngine.hpp"

class ImGuiDemoWindow : public GameEngine::ScriptableEntity {
public:
    void onImGui();

    std::string &imGuiName() override;

private:
    bool m_showDemoWindow = true;
};
