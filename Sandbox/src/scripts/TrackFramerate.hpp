#pragma once

#include "GameEngine.hpp"

class TrackFramerate : public GameEngine::ScriptableEntity {
public:
    void onImGui();

    std::string &imGuiName() override;

private:
    GameEngine::RollingAverage m_fpsRollingAverage;
};
