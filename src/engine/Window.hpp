#pragma once

#include <functional>
#include <GLFW/glfw3.h>

class Window {
public:
    void init(const std::function<void()> &rerenderRequiredCallback);

    bool shouldClose();

    bool onUpdate();

private:
    GLFWwindow *m_glfwWindow = nullptr;
    int m_renderSurfaceWidth = 960;
    int m_renderSurfaceHeight = 540;

    static double getWindowTime();

    friend class Time;
};
