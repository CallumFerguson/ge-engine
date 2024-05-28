#include "App.hpp"
#include "../rendering/backends/webgpu/WebGPURenderer.hpp"
#include "Time.hpp"
#include <iostream>

#ifdef __EMSCRIPTEN__

#include <emscripten.h>

#endif

Scene &App::getActiveScene() {
    return m_scene;
}

void App::onUpdate() {
    getActiveScene().onUpdate();
}

static std::function<void()> mainLoop;

#ifdef __EMSCRIPTEN__

static void mainLoopEmscripten() {
    try {
        mainLoop();
    } catch (const std::exception &e) {
        std::cout << "Caught exception:" << std::endl;
        std::cerr << e.what() << std::endl;
        emscripten_cancel_main_loop();
    } catch (...) {
        std::cerr << "Caught an unknown exception" << std::endl;
        emscripten_cancel_main_loop();
    }
}

#endif

void App::run() {
    m_window.init();
    WebGPURenderer::init();

    mainLoop = [&]() {
        if (!WebGPURenderer::initFinished()) {
            return;
        }
        if (!WebGPURenderer::initSuccessful()) {
            throw std::runtime_error("failed to initialize WebGPURenderer");
        }

        Time::onUpdate();
        m_window.onUpdate();
        onUpdate();
    };

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(mainLoopEmscripten, 0, true);
#else
    while (!m_window.shouldClose()) {
        try {
            mainLoop();
        } catch (const std::exception &e) {
            std::cout << "Caught exception:" << std::endl;
            std::cerr << e.what() << std::endl;
            break;
        } catch (...) {
            std::cerr << "Caught an unknown exception" << std::endl;
            break;
        }
    }
#endif
}
