#include "Exit.hpp"

#include <cstdlib>
#include <GLFW/glfw3.h>

#ifdef __EMSCRIPTEN__

#include <emscripten.h>
#include "../Utility/emscriptenUtility.hpp"

#endif

namespace GameEngine {

void exitApp(const std::string &message) {
    std::cout << "app exiting with message:" << std::endl;
    std::cout << message << std::endl;
    std::cout << "exiting..." << std::endl;

#ifdef __EMSCRIPTEN__
    emscripten_cancel_main_loop();
    glfwTerminate();
    resetCanvas();
    emscripten_force_exit(EXIT_FAILURE);
#else
    glfwTerminate();
    std::exit(EXIT_FAILURE);
#endif
}

}
