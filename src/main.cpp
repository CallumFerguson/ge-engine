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

// Define these only in *one* .cc file.
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.
#ifdef __EMSCRIPTEN__
#define TINYGLTF_NO_FS

#include <fstream>
#include "tiny_gltf_http_fs.hpp"

#endif

#include <tiny_gltf.h>

#include <cstdio>
#include <cstring>

#ifdef __EMSCRIPTEN__

#include <emscripten/fetch.h>

#endif

void loadModelAndPrintVertexCount(const std::string &filename) {
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    // Load the GLTF model from file
    bool result = loader.LoadBinaryFromFile(&model, &err, &warn, filename);
    if (!warn.empty()) {
        std::cout << "Warning: " << warn << std::endl;
    }
    if (!err.empty()) {
        std::cerr << "Error: " << err << std::endl;
    }
    if (!result) {
        std::cerr << "Failed to load GLTF model." << std::endl;
        return;
    }

    // Check if the model contains any meshes
    if (model.meshes.empty()) {
        std::cout << "Model does not contain any meshes." << std::endl;
        return;
    }

    // Access the first mesh
    const tinygltf::Mesh &mesh = model.meshes[0];
    if (mesh.primitives.empty()) {
        std::cout << "First mesh does not contain any primitives." << std::endl;
        return;
    }

    // Access the first primitive of the first mesh
    const tinygltf::Primitive &primitive = mesh.primitives[0];
    auto it = primitive.attributes.find("POSITION");
    if (it == primitive.attributes.end()) {
        std::cout << "Primitive does not contain POSITION attribute." << std::endl;
        return;
    }

    // Get the number of vertices from the POSITION accessor
    const tinygltf::Accessor &accessor = model.accessors[it->second];
    std::cout << "Number of vertices in the first mesh: " << accessor.count << std::endl;
}

#ifdef __EMSCRIPTEN__

#include <lib_webgpu.h>

WGpuAdapter adapter;
WGpuDevice device;

void ObtainedWebGpuDevice(WGpuDevice result, void *userData) {
    device = result;

    if (!device) {
        std::cout << "could not get device" << std::endl;
    }

    std::cout << "got device" << std::endl;
    std::cout << device << std::endl;
}

void ObtainedWebGpuAdapter(WGpuAdapter result, void *userData) {
    adapter = result;

    if (!adapter) {
        std::cout << "could not get adapter" << std::endl;
    }

    std::cout << "got adapter" << std::endl;
    std::cout << adapter << std::endl;

    WGpuDeviceDescriptor deviceDesc = {};

    WGpuSupportedLimits requiredLimits = {};

    deviceDesc.requiredLimits = requiredLimits;
    wgpu_adapter_request_device_async(adapter, &deviceDesc, ObtainedWebGpuDevice, nullptr);
}

void downloadSucceeded(emscripten_fetch_t *fetch) {
    printf("Finished downloading %llu bytes from URL %s.\n", fetch->numBytes, fetch->url);

    std::cout << fetch->data << std::endl;

    // The data is now available at fetch->data[0] through fetch->data[fetch->numBytes-1];
    emscripten_fetch_close(fetch); // Free data associated with the fetch.
}

void downloadFailed(emscripten_fetch_t *fetch) {
    printf("Downloading %s failed, HTTP failure status code: %d.\n", fetch->url, fetch->status);
    emscripten_fetch_close(fetch); // Also free data on failure.
}

#endif

int main() {
    std::cout << "main start" << std::endl;

    std::cout << "start" << std::endl;
#ifdef __EMSCRIPTEN__
    size_t fileSize;
    GetFileSizeInBytes(&fileSize, nullptr, "sphere.glb", nullptr);
    std::cout << fileSize << std::endl;
#endif
    std::cout << "finish" << std::endl;

    glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, 4.0f / 3.0f, 0.1f, 100.f);
    std::cout << Projection[0][0] << std::endl;

    std::cout << glm::pi<float>() << std::endl;

    loadModelAndPrintVertexCount("sphere.glb");

#ifdef __EMSCRIPTEN__
    if (navigator_gpu_available()) {
        std::cout << "WebGPU supported" << std::endl;
    } else {
        std::cout << "This browser does not support WebGPU" << std::endl;
        return -1;
    }

    WGpuRequestAdapterOptions options = {};
    options.powerPreference = WGPU_POWER_PREFERENCE_HIGH_PERFORMANCE;
    navigator_gpu_request_adapter_async(&options, ObtainedWebGpuAdapter, nullptr);
#endif

    std::cout << "done" << std::endl;

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
