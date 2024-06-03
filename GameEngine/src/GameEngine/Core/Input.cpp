#include "Input.hpp"

#include <unordered_map>
#include <GLFW/glfw3.h>

namespace GameEngine {

Window *Input::s_window;

static std::unordered_map<KeyCode, bool> s_keyStates;
static std::unordered_map<KeyCode, bool> s_previousKeyStates;

bool Input::getKey(KeyCode keyCode) {
    return glfwGetKey(s_window->m_glfwWindow, static_cast<int>(keyCode));
}

bool Input::getKeyDown(KeyCode keyCode) {
    bool currentState = getKey(keyCode);
    bool keyDown = currentState && currentState != s_previousKeyStates[keyCode];

    s_keyStates[keyCode] = currentState;

    return keyDown;
}

void Input::swapKeyStates() {
    s_previousKeyStates = s_keyStates;
    s_keyStates.clear();
}

}
