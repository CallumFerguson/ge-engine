#pragma once

#include "GameEngine.hpp"

class CameraController : public GameEngine::ScriptableEntity {
public:
    void onUpdate();

    void initFromJSON(const nlohmann::json &scriptJSON);

    [[nodiscard]] const char *objectName() const override {
        return "CameraController";
    }
};
