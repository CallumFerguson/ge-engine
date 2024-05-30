#pragma once

#include "../../../src/engine/ScriptableEntity.hpp"

class ImGuiDemoWindow : public ScriptableEntity {
public:
    void onImGui();

private:
    bool m_showDemoWindow = true;
};
