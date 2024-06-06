#pragma once

#include "GameEngine.hpp"

class Rotator : public GameEngine::ScriptableEntity {
public:
    float speed = 360.0f;

    void onUpdate();

    void onImGuiInspector() override;

    nlohmann::json toJSON();

    [[nodiscard]] const char *objectName() const override {
        return "Rotator";
    }
};
