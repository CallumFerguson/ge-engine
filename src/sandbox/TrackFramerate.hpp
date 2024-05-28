#pragma once

#include "../engine/ScriptableEntity.hpp"
#include "../utility/RollingAverage.hpp"

class TrackFramerate : public ScriptableEntity {
public:
    void onUpdate();

private:
    RollingAverage m_fpsRollingAverage;
};
