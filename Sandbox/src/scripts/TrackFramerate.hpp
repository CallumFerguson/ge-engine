#pragma once

#include "GameEngine.hpp"

class TrackFramerate : public GameEngine::ScriptableEntity {
public:
    void onImGui();

    [[nodiscard]] const char *objectName() const override {
        return "TrackFramerate";
    }

private:
    GameEngine::RollingAverage m_fpsRollingAverage;
};
