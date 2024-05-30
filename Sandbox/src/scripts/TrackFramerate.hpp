#pragma once

#include "GameEngine.hpp"

class TrackFramerate : public ScriptableEntity {
public:
    void onImGui();

private:
    RollingAverage m_fpsRollingAverage;
};
