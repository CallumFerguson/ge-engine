#pragma once

#include "GameEngine.hpp"

class CameraController : public GameEngine::ScriptableEntity {
public:
    void onUpdate();

    std::string &imGuiName() override;
};
