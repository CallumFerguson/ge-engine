#pragma once

#include "../engine/ScriptableEntity.hpp"
#include "../utility/RollingAverage.hpp"

class TrackFramerate : public ScriptableEntity {
public:
    void onImGui();

private:
    RollingAverage m_fpsRollingAverage;
};
