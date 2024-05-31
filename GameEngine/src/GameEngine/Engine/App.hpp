#pragma once

#include "../Scene/Scene.hpp"
#include "Window.hpp"

class App {
public:
    App();

    void run();

    Scene &getActiveScene();

private:
    Scene m_scene;
    Window m_window;
};
