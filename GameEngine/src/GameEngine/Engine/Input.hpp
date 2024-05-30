#pragma once

#include "KeyCode.hpp"
#include "Window.hpp"

class Input {
public:
    static bool getKey(KeyCode keyCode);

    static bool getKeyDown(KeyCode keyCode);

private:
    static Window *s_window;

    friend class Window;
};
