#pragma once

#include "Scene.hpp"
#include "Window.hpp"

class App {
public:
    void init();

    void run();

    Scene &getActiveScene();

private:
    Scene m_scene;
    Window m_window;
};
