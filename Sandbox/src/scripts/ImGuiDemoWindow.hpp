#pragma once

#include "GameEngine.hpp"

class ImGuiDemoWindow : public GameEngine::ScriptableEntity {
public:
    void onImGui();

private:
    bool m_showDemoWindow = true;
};
