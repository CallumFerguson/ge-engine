#pragma once

#include "../Scene/Scene.hpp"
#include "Window.hpp"

namespace GameEngine {

class App {
public:
    App(const std::string &windowTitle);

    void run();

    Scene &getActiveScene();

private:
    Scene m_scene;
    Window m_window;

    void registerComponents();
};

}
