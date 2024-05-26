#pragma once

#include "../engine/ScriptableEntity.hpp"
#include "../engine/Components.hpp"

struct TestScript : public ScriptableEntity {
    void onFirstUpdate();

    void onUpdate();
};