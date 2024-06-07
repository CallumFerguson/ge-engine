#pragma once

#include "GameEngine.hpp"

class PBRColorRandomizer : public GameEngine::ScriptableEntity {
public:
    void onStart();

    void onUpdate();

    [[nodiscard]] const char *objectName() const override {
        return "PBRColorRandomizer";
    }

private:
    void randomizeColor();
};
