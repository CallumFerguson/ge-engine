#pragma once

#include <functional>
#include <GLFW/glfw3.h>

namespace GameEngine {

class WebGPURenderer;

class Window {
public:
    Window();

    static Window &mainWindow();

    void init(const std::function<void()> &rerenderRequiredCallback, const std::string &windowTitle);

    bool shouldClose();

    bool onUpdate();

    [[nodiscard]] int renderSurfaceWidth() const;

    [[nodiscard]] int renderSurfaceHeight() const;

    [[nodiscard]] float aspectRatio() const;

    void addDropCallback(std::function<void(std::vector<std::filesystem::path>& paths)> callback);

private:
    GLFWwindow *m_glfwWindow = nullptr;
    int m_renderSurfaceWidth = 1280;
    int m_renderSurfaceHeight = 720;

    static double getWindowTime();

    static void scrollCallback(GLFWwindow *window, double xoffset, double yoffset);

    static void dropCallback(GLFWwindow *window, int count, const char **paths);

    friend class Time;

    friend class WebGPURenderer;

    friend class Input;
};

}
