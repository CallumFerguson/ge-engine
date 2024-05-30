#pragma once

#include "../../src/engine/ScriptableEntity.hpp"
#include "../../src/utility/RollingAverage.hpp"

class TrackFramerate : public ScriptableEntity {
public:
    void onImGui();

private:
    RollingAverage m_fpsRollingAverage;
};
