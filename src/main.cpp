#include <iostream>
#include <string>

#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/scalar_constants.hpp>

// Define these only in *one* .cc file.
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.
#include <tiny_gltf.h>

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

#endif

int main() {
    std::cout << "main start" << std::endl;

    try {
        throw std::runtime_error("test error");
    } catch (const std::runtime_error &e) {
        std::cout << "caught error: " << e.what() << std::endl;
    }

    std::ifstream file("assets/sphere.glb", std::ifstream::ate | std::ifstream::binary);
    if (file.is_open()) {
        std::cout << file.tellg() << std::endl;
    } else {
        std::cout << "cannot open file" << std::endl;
    }

    glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, 4.0f / 3.0f, 0.1f, 100.f);
    std::cout << Projection[0][0] << std::endl;

    std::cout << glm::pi<float>() << std::endl;

    loadModelAndPrintVertexCount("assets/sphere.glb");

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

    auto canvasContext = wgpu_canvas_get_webgpu_context("canvas");
    if (!wgpu_is_canvas_context(canvasContext)) {
        std::cout << "could not get context" << std::endl;
        return -1;
    }
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
