#include "App.hpp"
#include "../rendering/backends/webgpu/WebGPURenderer.hpp"
#include "Time.hpp"

Scene &App::getActiveScene() {
    return m_scene;
}

void App::onUpdate() {
    getActiveScene().onUpdate();
}

void App::run() {
    m_window.init();

    WebGPURenderer::init([&](bool success) {
        try {
            if (success) {
                runMainLoop();
            } else {
                throw std::runtime_error("failed to init WebGPURenderer");
            }
        } catch (const std::exception &e) {
            std::cout << "Caught exception:" << std::endl;
            std::cerr << e.what() << std::endl;
        } catch (...) {
            std::cerr << "Caught an unknown exception" << std::endl;
        }
    });
}

void App::runMainLoop() {
    while (!m_window.shouldClose()) {
        Time::onUpdate();
        m_window.onUpdate();
        onUpdate();
    }
}
