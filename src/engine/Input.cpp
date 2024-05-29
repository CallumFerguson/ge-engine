#include "Input.hpp"

#include <unordered_map>
#include <GLFW/glfw3.h>

Window *Input::s_window;

static std::unordered_map<KeyCode, bool> s_previousKeyStates;

bool Input::getKey(KeyCode keyCode) {
    return glfwGetKey(s_window->m_glfwWindow, static_cast<int>(keyCode));
}

bool Input::getKeyDown(KeyCode keyCode) {
    bool currentState = getKey(keyCode);
    bool previousState = s_previousKeyStates[keyCode];
    s_previousKeyStates[keyCode] = currentState;
    return currentState && !previousState;
}
