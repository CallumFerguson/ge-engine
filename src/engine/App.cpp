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

static void mainLoopStatic() {
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

#else

// the main loop for native builds does not need to be static, but since Emscripten does require a static callback,
// also make the native main loop static to keep code cleaner
static bool mainLoopStatic() {
    try {
        mainLoop();
        return true;
    } catch (const std::exception &e) {
        std::cout << "Caught exception:" << std::endl;
        std::cerr << e.what() << std::endl;
        return false;
    } catch (...) {
        std::cerr << "Caught an unknown exception" << std::endl;
        return false;
    }
}

#endif

void App::run() {
    m_window.init(mainLoopStatic);
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
    emscripten_set_main_loop(mainLoopStatic, 0, true);
#else
    while (!m_window.shouldClose()) {
        if (!mainLoopStatic()) {
            break;
        }
    }
#endif
}
