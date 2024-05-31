#pragma once

#include "GameEngine.hpp"

struct TestScript : public GameEngine::ScriptableEntity {
    void onStart();

    void onUpdate();

    void onImGui();
};
