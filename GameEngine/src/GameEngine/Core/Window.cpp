#include "Window.hpp"

#include <iostream>
#include <functional>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_wgpu.h>
#include "Input.hpp"
#include "../Utility/utility.hpp"
#include "../Rendering/Backends/WebGPU/WebGPURenderer.hpp"

#ifdef __EMSCRIPTEN__

#include <emscripten.h>

#endif

namespace GameEngine {

static Window *s_mainWindow;
static std::function<void()> s_rerenderRequiredCallback;

#ifndef __EMSCRIPTEN__

static void windowPosCallback(GLFWwindow *, int, int) {
    s_rerenderRequiredCallback();
}

static void framebufferSizeCallback(GLFWwindow *, int, int) {
    s_rerenderRequiredCallback();
}

#endif

void Window::init(const std::function<void()> &rerenderRequiredCallback) {
    s_rerenderRequiredCallback = rerenderRequiredCallback;

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

#ifndef __EMSCRIPTEN__
    setWindowIcon(m_glfwWindow, "assets/app-icon.png");
#endif

    // in a browser, glfwCreateWindow will ignore the passed width and height,
    // so get the actual size which is based on the canvas size
#ifdef __EMSCRIPTEN__
    glfwGetWindowSize(m_glfwWindow, &m_renderSurfaceWidth, &m_renderSurfaceHeight);
#endif

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    // this is only needed for emscripten, but to keep things consistent, just do it for all platforms.
    // if init file is needed later, it can be added to emscripten by preloading/file packing it
    io.IniFilename = nullptr;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOther(m_glfwWindow, true);

#ifdef __EMSCRIPTEN__
    ImGui_ImplGlfw_InstallEmscriptenCanvasResizeCallback("#canvas");
#else
    glfwSetWindowPosCallback(m_glfwWindow, windowPosCallback);
    glfwSetFramebufferSizeCallback(m_glfwWindow, framebufferSizeCallback);
#endif

    Input::s_window = this;
}

bool Window::shouldClose() {
    return glfwWindowShouldClose(m_glfwWindow);
}

bool Window::onUpdate() {
    glfwPollEvents();

    int currentRenderSurfaceWidth, currentRenderSurfaceHeight;
    glfwGetFramebufferSize(m_glfwWindow, &currentRenderSurfaceWidth, &currentRenderSurfaceHeight);
    if (currentRenderSurfaceWidth == 0 || currentRenderSurfaceHeight == 0) {
        // TODO: maybe Engine should keep running but just not render size is invalid or window is minimized
        return false;
    }
    if (currentRenderSurfaceWidth != m_renderSurfaceWidth || currentRenderSurfaceHeight != m_renderSurfaceHeight) {
        ImGui_ImplWGPU_InvalidateDeviceObjects();
        m_renderSurfaceWidth = currentRenderSurfaceWidth;
        m_renderSurfaceHeight = currentRenderSurfaceHeight;
        WebGPURenderer::configureSurface();
        ImGui_ImplWGPU_CreateDeviceObjects();
    }

    return true;
}

int Window::renderSurfaceWidth() const {
    return m_renderSurfaceWidth;
}

int Window::renderSurfaceHeight() const {
    return m_renderSurfaceHeight;
}

double Window::getWindowTime() {
    return glfwGetTime();
}

const Window &Window::mainWindow() {
    return *s_mainWindow;
}

Window::Window() {
    s_mainWindow = this;
}

float Window::aspectRatio() const {
    return static_cast<float>(m_renderSurfaceWidth) / static_cast<float>(m_renderSurfaceHeight);
}

}
