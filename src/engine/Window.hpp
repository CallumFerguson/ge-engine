#pragma once

#include <functional>
#include <GLFW/glfw3.h>

class Window {
public:
    void init(const std::function<void()> &rerenderRequiredCallback);

    bool shouldClose();

    void onUpdate();

private:
    GLFWwindow *m_glfwWindow = nullptr;
    int m_renderSurfaceWidth = 512;
    int m_renderSurfaceHeight = 512;

    static double getWindowTime();

    friend class Time;
};
