#pragma once

#include "GameEngine.hpp"

class Rotator : public GameEngine::ScriptableEntity {
public:
    void onUpdate();

    float speed = 1.0f;
};
