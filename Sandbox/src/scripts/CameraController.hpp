#pragma once

#include "GameEngine.hpp"

class CameraController : public GameEngine::ScriptableEntity {
public:
    void onUpdate();

    nlohmann::json toJSON();

    [[nodiscard]] const char *objectName() const override {
        return "CameraController";
    }
};
