#pragma once

#include <GLFW/glfw3.h>

class Window {
public:
    Window();

    bool shouldClose();

    void onUpdate();

private:
    GLFWwindow *m_glfwWindow = nullptr;
    int m_renderSurfaceWidth = 512;
    int m_renderSurfaceHeight = 512;
};
