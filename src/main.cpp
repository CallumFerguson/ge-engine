#include <iostream>

#ifdef __EMSCRIPTEN__

#include "webgpu.hpp"

#endif

int main() {
    std::cout << "main start" << std::endl;

#ifdef __EMSCRIPTEN__
    webgpuTest();
#endif

    std::cout << "done" << std::endl;
}
