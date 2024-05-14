#include <iostream>

#include "gltfloader.hpp"
#include "webGPU.hpp"

int main() {
    std::cout << "main start" << std::endl;

    loadModelAndPrintVertexCount("assets/sphere.glb");

    webGPUTest();

    std::cout << "done" << std::endl;
}
