#pragma once

#include "Scene.hpp"

class App {
public:
    Scene &getActiveScene();

    void run();

private:
    Scene m_scene;
};
