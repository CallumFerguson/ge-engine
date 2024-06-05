#pragma once

#include "GameEngine.hpp"

class Rotator : public GameEngine::ScriptableEntity {
public:
    float speed = 360.0f;

    void onUpdate();

    std::string &imGuiName() override;

    void onImGuiInspector() override;
};
