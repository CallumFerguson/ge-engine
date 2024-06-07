#pragma once

#include "GameEngine.hpp"

class TrackFramerate : public GameEngine::ScriptableEntity {
public:
    void onImGui();

    nlohmann::json toJSON();

    void initFromJSON(const nlohmann::json &scriptJSON);

    [[nodiscard]] const char *objectName() const override {
        return "TrackFramerate";
    }

private:
    GameEngine::RollingAverage m_fpsRollingAverage;
};
