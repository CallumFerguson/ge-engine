#include "App.hpp"
#include "../rendering/backends/webgpu/WebGPURenderer.hpp"
#include "Time.hpp"
#include <iostream>

#ifdef __EMSCRIPTEN__

#include <emscripten.h>
#include "../utility/emscriptenUtility.hpp"

#endif

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

App::App() {
    m_window.init(mainLoopStatic);
    WebGPURenderer::init(&m_window);
}

void App::run() {
    mainLoop = [&]() {
        if (!m_window.onUpdate()) {
            return;
        }

        Time::onUpdate();

        WebGPURenderer::startFrame();

        getActiveScene().onUpdate();

        WebGPURenderer::endFrame();

#ifdef __EMSCRIPTEN__
        updateCursor();
#endif

        WebGPURenderer::present();
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

Scene &App::getActiveScene() {
    return m_scene;
}
