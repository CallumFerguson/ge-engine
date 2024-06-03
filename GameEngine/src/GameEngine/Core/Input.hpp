#pragma once

#include "KeyCode.hpp"
#include "Window.hpp"

namespace GameEngine {

class Input {
public:
    static bool getKey(KeyCode keyCode);

    static bool getKeyDown(KeyCode keyCode);

private:
    static Window *s_window;

    static void swapKeyStates();

    friend class Window;
};

}
