#pragma once

#include "GameEngine.hpp"

class ImGuiDemoWindow : public GameEngine::ScriptableEntity {
public:
    void onImGui();

    [[nodiscard]] const char *objectName() const override {
        return "ImGuiDemoWindow";
    }

private:
    bool m_showDemoWindow = true;
};
