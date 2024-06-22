#include "Input.hpp"

#include <limits>
#include <cmath>
#include <unordered_map>
#include <GLFW/glfw3.h>

namespace GameEngine {

Window *Input::s_window;

static float s_mousePositionX = 0, s_mousePositionY = 0;
static float s_lastMousePositionX = std::numeric_limits<float>::quiet_NaN(), s_lastMousePositionY = std::numeric_limits<float>::quiet_NaN();
static float s_mouseDeltaX = 0, s_mouseDeltaY = 0;

static bool s_mouseButtonLeft = false, s_mouseButtonRight = false, s_mouseButtonMiddle = false;
static bool s_lastMouseButtonLeft = false, s_lastMouseButtonRight = false, s_lastMouseButtonMiddle = false;
static bool s_mouseButtonLeftDown = false, s_mouseButtonRightDown = false, s_mouseButtonMiddleDown = false;

static std::unordered_map<KeyCode, bool> s_keyStates;
static std::unordered_map<KeyCode, bool> s_previousKeyStates;

static float s_accumulatedWheelDeltaX = 0, s_accumulatedWheelDeltaY = 0;
static float s_wheelDeltaX = 0, s_wheelDeltaY = 0;

bool Input::getKey(KeyCode keyCode) {
    return glfwGetKey(s_window->m_glfwWindow, static_cast<int>(keyCode));
}

bool Input::getKeyDown(KeyCode keyCode) {
    bool currentState = getKey(keyCode);
    bool keyDown = currentState && currentState != s_previousKeyStates[keyCode];

    s_keyStates[keyCode] = currentState;

    return keyDown;
}

void Input::onUpdate() {
    swapKeyStates();

    s_mouseButtonLeft = glfwGetMouseButton(s_window->m_glfwWindow, GLFW_MOUSE_BUTTON_LEFT);
    s_mouseButtonRight = glfwGetMouseButton(s_window->m_glfwWindow, GLFW_MOUSE_BUTTON_RIGHT);
    s_mouseButtonMiddle = glfwGetMouseButton(s_window->m_glfwWindow, GLFW_MOUSE_BUTTON_MIDDLE);

    s_mouseButtonLeftDown = s_mouseButtonLeft && !s_lastMouseButtonLeft;
    s_mouseButtonRightDown = s_mouseButtonRight && !s_lastMouseButtonRight;
    s_mouseButtonMiddleDown = s_mouseButtonMiddle && !s_lastMouseButtonMiddle;

    s_lastMouseButtonLeft = s_mouseButtonLeft;
    s_lastMouseButtonRight = s_mouseButtonRight;
    s_lastMouseButtonMiddle = s_mouseButtonMiddle;

    double x, y;
    glfwGetCursorPos(s_window->m_glfwWindow, &x, &y);
    s_mousePositionX = static_cast<float>(x);
    s_mousePositionY = static_cast<float>(y);

    if (!std::isnan(s_lastMousePositionX)) {
        s_mouseDeltaX = s_lastMousePositionX - s_mousePositionX;
    }
    if (!std::isnan(s_lastMousePositionY)) {
        s_mouseDeltaY = s_lastMousePositionY - s_mousePositionY;
    }

    s_lastMousePositionX = s_mousePositionX;
    s_lastMousePositionY = s_mousePositionY;

    s_wheelDeltaX = s_accumulatedWheelDeltaX;
    s_wheelDeltaY = s_accumulatedWheelDeltaY;

    s_accumulatedWheelDeltaX = 0;
    s_accumulatedWheelDeltaY = 0;
}

void Input::swapKeyStates() {
    s_previousKeyStates = s_keyStates;
    s_keyStates.clear();
}

float Input::mousePositionX() {
    return s_mousePositionX;
}

float Input::mousePositionY() {
    return s_mousePositionY;
}

float Input::mouseDeltaX() {
    return s_mouseDeltaX;
}

float Input::mouseDeltaY() {
    return s_mouseDeltaY;
}

bool Input::getMouseButton(Input::MouseButton mouseButton) {
    switch (mouseButton) {
        case MouseButton::left:
            return s_mouseButtonLeft;
        case MouseButton::right:
            return s_mouseButtonRight;
        case MouseButton::middle:
            return s_mouseButtonMiddle;
        default:
            std::cout << "unknown getMouseButton button: " << (int) mouseButton << std::endl;
            return false;
    }
}

bool Input::getMouseButtonDown(Input::MouseButton mouseButton) {
    switch (mouseButton) {
        case MouseButton::left:
            return s_mouseButtonLeftDown;
        case MouseButton::right:
            return s_mouseButtonRightDown;
        case MouseButton::middle:
            return s_mouseButtonMiddleDown;
        default:
            std::cout << "unknown getMouseButtonDown button: " << (int) mouseButton << std::endl;
            return false;
    }
}

void Input::scrollCallback(float deltaX, float deltaY) {
    s_accumulatedWheelDeltaX += deltaX;
    s_accumulatedWheelDeltaY += deltaY;
}

float Input::wheelDeltaX() {
    return s_wheelDeltaX;
}

float Input::wheelDeltaY() {
    return s_wheelDeltaY;
}

}
