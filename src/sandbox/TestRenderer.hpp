#pragma once

#include "../engine/ScriptableEntity.hpp"

struct TestRenderer : public ScriptableEntity {
    void onStart();

    void onUpdate();

    void onRender();
};
