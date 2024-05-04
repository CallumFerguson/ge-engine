#include <iostream>

#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/scalar_constants.hpp>

//#include <GLFW/glfw3.h>

//#include <emscripten.h>
//#include <emscripten/html5.h>

//// @formatter:off
//EM_JS(void, initializeWebGPU, (), {
//    (async() => {
//        const canvas = document.querySelector('canvas');
//        const adapter = await navigator.gpu.requestAdapter();
//        const device = await adapter.requestDevice();
//        console.log(device);
//    })();
//})
//// @formatter:on

#include <lib_webgpu.h>

WGpuAdapter adapter;

void ObtainedWebGpuAdapter(WGpuAdapter result, void *userData) {
    adapter = result;

    std::cout << "got adapter" << std::endl;
    std::cout << adapter << std::endl;
}

int main() {
    std::cout << "test" << std::endl;

    glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, 4.0f / 3.0f, 0.1f, 100.f);
    std::cout << Projection[0][0] << std::endl;

    std::cout << glm::pi<float>() << std::endl;

//    initializeWebGPU();

    printf("start\n");

    WGpuRequestAdapterOptions options = {};
    options.powerPreference = WGPU_POWER_PREFERENCE_HIGH_PERFORMANCE;
    if (navigator_gpu_request_adapter_async(&options, ObtainedWebGpuAdapter, nullptr)) {
        std::cout << "WebGPU supported" << std::endl;
    } else {
        std::cout << "This browser does not support WebGPU" << std::endl;
    }

//    WGPUInstanceDescriptor desc = {};
//    desc.nextInChain = nullptr;
//    printf("1\n");
//
//    WGPUInstance instance = wgpuCreateInstance(&desc);
//    printf("2\n");
//    if (!instance) {
//        printf("Failed to create WebGPU instance\n");
//        return -1;
//    }

    printf("done\n");

//    if (!glfwInit()) {
//        return -1;
//    }
//
//    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
//
//    GLFWwindow *window = glfwCreateWindow(640, 480, "Game Engine", NULL, NULL);
//    if (!window) {
//        glfwTerminate();
//        return -1;
//    }
//
//    while (!glfwWindowShouldClose(window)) {
//        glfwPollEvents();
//    }
//
//    glfwTerminate();
}
