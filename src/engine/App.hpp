#pragma once

#include "Scene.hpp"
#include "Window.hpp"

class App {
public:
    Scene &getActiveScene();

    void run();

    void onUpdate();

private:
    Scene m_scene;
    Window m_window;

    void runMainLoop();
};
