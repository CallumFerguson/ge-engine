#pragma once

#include "GameEngine.hpp"

class TrackFramerate : public GameEngine::ScriptableEntity {
public:
    void onImGui();

private:
    GameEngine::RollingAverage m_fpsRollingAverage;
};
