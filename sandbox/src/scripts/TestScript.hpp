#pragma once

#include "GameEngine.hpp"

struct TestScript : public ScriptableEntity {
    void onStart();

    void onUpdate();

    void onImGui();
};
