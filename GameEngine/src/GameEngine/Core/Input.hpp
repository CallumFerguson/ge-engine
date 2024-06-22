#pragma once

#include "KeyCode.hpp"
#include "Window.hpp"

namespace GameEngine {

class Input {
public:
    enum class MouseButton {
        left,
        right,
        middle
    };

    static bool getKey(KeyCode keyCode);

    static bool getKeyDown(KeyCode keyCode);

    static float mousePositionX();

    static float mousePositionY();

    static float mouseDeltaX();

    static float mouseDeltaY();

    static float wheelDeltaX();

    static float wheelDeltaY();

    static bool getMouseButton(MouseButton mouseButton);

    static bool getMouseButtonDown(MouseButton mouseButton);

private:
    static Window *s_window;

    static void onUpdate();

    static void swapKeyStates();

    static void scrollCallback(float deltaX, float deltaY);

    friend class Window;
};

}
