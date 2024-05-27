#include "Window.hpp"

#include <iostream>

void Window::init() {
    if (!glfwInit()) {
        std::cout << "could not glfwInit" << std::endl;
        return;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_glfwWindow = glfwCreateWindow(m_renderSurfaceWidth, m_renderSurfaceHeight, "Game Engine", nullptr, nullptr);
    if (!m_glfwWindow) {
        std::cout << "failed to create window" << std::endl;
        return;
    }
}

bool Window::shouldClose() {
    return glfwWindowShouldClose(m_glfwWindow);
}

void Window::onUpdate() {
    glfwPollEvents();

    // React to changes in screen size
    int currentRenderSurfaceWidth, currentRenderSurfaceHeight;
    glfwGetFramebufferSize(m_glfwWindow, &currentRenderSurfaceWidth, &currentRenderSurfaceHeight);
    if (currentRenderSurfaceWidth == 0 || currentRenderSurfaceHeight == 0) {
        std::cout << "TODO: don't render if surface is 0 width or height. maybe also if minimized" << std::endl;
        return;
    }
    if (currentRenderSurfaceWidth != m_renderSurfaceWidth || currentRenderSurfaceHeight != m_renderSurfaceHeight) {
//        ImGui_ImplWGPU_InvalidateDeviceObjects();
        m_renderSurfaceWidth = currentRenderSurfaceWidth;
        m_renderSurfaceHeight = currentRenderSurfaceHeight;
//        configureSurface();
//        ImGui_ImplWGPU_CreateDeviceObjects();
    }
}

double Window::getWindowTime() {
    return glfwGetTime();
}
