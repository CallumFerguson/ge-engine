#pragma once

#include "../Scene/Scene.hpp"
#include "Window.hpp"

namespace GameEngine {

class App {
public:
    App();

    ~App() {
        std::cout << "app bye" << std::endl;
        
    }

    void run();

    Scene &getActiveScene();

private:
    Scene m_scene;
    Window m_window;
};

}
