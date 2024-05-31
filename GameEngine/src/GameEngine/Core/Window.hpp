#pragma once

#include <functional>
#include <GLFW/glfw3.h>

namespace GameEngine {

class WebGPURenderer;

class Window {
public:
    void init(const std::function<void()> &rerenderRequiredCallback);

    bool shouldClose();

    bool onUpdate();

    [[nodiscard]] int renderSurfaceWidth() const;

    [[nodiscard]] int renderSurfaceHeight() const;

private:
    GLFWwindow *m_glfwWindow = nullptr;
    int m_renderSurfaceWidth = 960;
    int m_renderSurfaceHeight = 540;

    static double getWindowTime();

    friend class Time;

    friend class WebGPURenderer;

    friend class Input;
};

}
