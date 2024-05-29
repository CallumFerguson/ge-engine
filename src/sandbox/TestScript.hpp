#pragma once

#include "../engine/ScriptableEntity.hpp"

struct TestScript : public ScriptableEntity {
    void onStart();

    void onUpdate();

    void onImGui();
};
