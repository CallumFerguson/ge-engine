#include "webGPU.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <webgpu/webgpu_cpp.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_wgpu.h>
#include <GLFW/glfw3.h>

#include <imgui_internal.h>

#ifdef __EMSCRIPTEN__

#include "webGPUEmscripten.hpp"
#include <emscripten.h>
#include <emscripten/html5_webgpu.h>

#else

#include "webGPUDawn.hpp"
#include <webgpu/webgpu_glfw.h>

#endif

wgpu::Instance instance;
wgpu::Adapter adapter;
wgpu::Device device;
wgpu::Surface surface;
wgpu::RenderPipeline pipeline;

wgpu::RenderPassColorAttachment colorAttachment;
wgpu::RenderPassDescriptor renderPassDescriptor;

wgpu::TextureFormat presentationFormat;

GLFWwindow* window;

#ifdef __EMSCRIPTEN__
wgpu::SwapChain swapChain;
#endif

bool showDemoWindow = true;

bool once = true;

int renderSurfaceWidth = 512;
int renderSurfaceHeight = 512;

void configureSurface() {
#ifdef __EMSCRIPTEN__
    wgpu::SwapChainDescriptor swapChainDescriptor = {};
    swapChainDescriptor.usage = wgpu::TextureUsage::RenderAttachment;
    swapChainDescriptor.format = presentationFormat;
    swapChainDescriptor.width = renderSurfaceWidth;
    swapChainDescriptor.height = renderSurfaceHeight;
    swapChainDescriptor.presentMode = wgpu::PresentMode::Fifo;
    swapChain = device.CreateSwapChain(surface, &swapChainDescriptor);
#else
    wgpu::SurfaceConfiguration surfaceConfiguration = {};
    surfaceConfiguration.device = device;
    surfaceConfiguration.format = presentationFormat;
    surfaceConfiguration.width = renderSurfaceWidth;
    surfaceConfiguration.height = renderSurfaceHeight;
    surface.Configure(&surfaceConfiguration);
#endif
}

void draw() {
    ImGui_ImplWGPU_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (showDemoWindow) {
        ImGui::ShowDemoWindow(&showDemoWindow);
    }

    ImGui::Render();

#ifndef __EMSCRIPTEN__
    // Tick needs to be called in Dawn to display validation errors
    // https://github.com/ocornut/imgui/blob/master/examples/example_glfw_wgpu/main.cpp
    device.Tick();
#endif

#ifdef __EMSCRIPTEN__
    auto currentSurfaceTextureView = swapChain.GetCurrentTextureView();
#else
    wgpu::SurfaceTexture currentSurfaceTexture;
    surface.GetCurrentTexture(&currentSurfaceTexture);
    auto currentSurfaceTextureView = currentSurfaceTexture.texture.CreateView();
#endif

    colorAttachment.view = currentSurfaceTextureView;

    auto commandEncoder = device.CreateCommandEncoder();

    auto renderPassEncoder = commandEncoder.BeginRenderPass(&renderPassDescriptor);

    renderPassEncoder.SetPipeline(pipeline);
    renderPassEncoder.Draw(3);

    ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), renderPassEncoder.Get());

    renderPassEncoder.End();

    auto commandBuffer = commandEncoder.Finish();
    device.GetQueue().Submit(1, &commandBuffer);
}

void mainLoop() {
    glfwPollEvents();

    // React to changes in screen size
    int currentRenderSurfaceWidth, currentRenderSurfaceHeight;
    glfwGetFramebufferSize(window, &currentRenderSurfaceWidth, &currentRenderSurfaceHeight);
    if (currentRenderSurfaceWidth != renderSurfaceWidth || currentRenderSurfaceHeight != renderSurfaceHeight) {
        ImGui_ImplWGPU_InvalidateDeviceObjects();
        renderSurfaceWidth = currentRenderSurfaceWidth;
        renderSurfaceHeight = currentRenderSurfaceHeight;
        configureSurface();
        ImGui_ImplWGPU_CreateDeviceObjects();
    }

    draw();

#ifdef __EMSCRIPTEN__
    updateCursor();
#endif

#ifndef __EMSCRIPTEN__
    surface.Present();
    instance.ProcessEvents();
#endif
}

#ifndef __EMSCRIPTEN__
void windowPosCallback(GLFWwindow*, int xpos, int ypos) {
    mainLoop();
}

void framebufferSizeCallback(GLFWwindow*, int width, int height) {
    mainLoop();
}
#endif

void mainWebGPU() {
    if (!glfwInit()) {
        std::cout << "could not glfwInit" << std::endl;
        return;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(renderSurfaceWidth, renderSurfaceHeight, "WebGPU window", nullptr, nullptr);
    if (!window) {
        std::cout << "failed to create window" << std::endl;
        return;
    }

    // in a browser, glfwCreateWindow will ignore the passed width and height,
    // so get the actual size which is based on the canvas size
#ifdef __EMSCRIPTEN__
    glfwGetWindowSize(window, &renderSurfaceWidth, &renderSurfaceHeight);
#endif

#ifdef __EMSCRIPTEN__
    wgpu::SurfaceDescriptorFromCanvasHTMLSelector canvasDescriptor = {};
    canvasDescriptor.selector = "#canvas";

    wgpu::SurfaceDescriptor surfaceDescriptor = {};
    surfaceDescriptor.nextInChain = &canvasDescriptor;

    surface = instance.CreateSurface(&surfaceDescriptor);
#else
    surface = wgpu::glfw::CreateSurfaceForWindow(instance, window);
#endif

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    // this is only needed for emscripten, but to keep things consistent, just do it for all platforms.
    // if init file is needed later, it can be added to emscripten by preloading/file packing it
    io.IniFilename = nullptr;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOther(window, true);

#ifdef __EMSCRIPTEN__
    ImGui_ImplGlfw_InstallEmscriptenCanvasResizeCallback("#canvas");
#else
    glfwSetWindowPosCallback(window, windowPosCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
#endif

    presentationFormat = surface.GetPreferredFormat(adapter);

    configureSurface();

    ImGui_ImplWGPU_InitInfo init_info;
    init_info.Device = device.Get();
    init_info.NumFramesInFlight = 3;
    init_info.RenderTargetFormat = (WGPUTextureFormat)presentationFormat;
    init_info.DepthStencilFormat = WGPUTextureFormat_Undefined;
    ImGui_ImplWGPU_Init(&init_info);

    std::ifstream shaderFile("shaders/simple_triangle.wgsl", std::ios::binary);
    if (!shaderFile) {
        throw std::runtime_error("Could not open shader file");
    }
    std::stringstream shaderBuffer;
    shaderBuffer << shaderFile.rdbuf();
    auto shaderString = shaderBuffer.str();

    wgpu::ShaderModuleWGSLDescriptor wgslDescriptor{};
    wgslDescriptor.code = shaderString.c_str();

    wgpu::ShaderModuleDescriptor shaderModuleDescriptor = {};
    shaderModuleDescriptor.nextInChain = &wgslDescriptor;
    auto shaderModule = device.CreateShaderModule(&shaderModuleDescriptor);

    wgpu::ColorTargetState colorTargetState = {};
    colorTargetState.format = presentationFormat;

    wgpu::FragmentState fragment = {};
    fragment.module = shaderModule;
    fragment.entryPoint = "frag";
    fragment.targetCount = 1;
    fragment.targets = &colorTargetState;

    wgpu::RenderPipelineDescriptor pipelineDescriptor = {};

    wgpu::VertexState vertex = {};
    vertex.module = shaderModule;
    vertex.entryPoint = "vert";
    vertex.bufferCount = 0;

    pipelineDescriptor.vertex = vertex;
    pipelineDescriptor.fragment = &fragment;

    pipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
    pipelineDescriptor.primitive.cullMode = wgpu::CullMode::Back;

    pipelineDescriptor.multisample.count = 1;

    pipeline = device.CreateRenderPipeline(&pipelineDescriptor);

    colorAttachment = {};
    colorAttachment.loadOp = wgpu::LoadOp::Clear;
    colorAttachment.storeOp = wgpu::StoreOp::Store;
    colorAttachment.clearValue = wgpu::Color{0, 0, 0, 1};

    renderPassDescriptor = {};
    renderPassDescriptor.colorAttachmentCount = 1;
    renderPassDescriptor.colorAttachments = &colorAttachment;

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(mainLoop, 0, false); // TODO: try simulate loop, also animation loop variables? or just use c++ stuff to get current time
#else
    while (!glfwWindowShouldClose(window)) {
        mainLoop();
    }
#endif
}

void errorCallback(WGPUErrorType type, const char *message, void *userdata) {
    std::ostringstream oss;
    oss << "Error: " << type << " - message: " << message;
    throw std::runtime_error(oss.str());
}

void getDeviceCallback(WGPURequestDeviceStatus status, WGPUDevice cDevice, const char *message, void *userdata) {
    device = wgpu::Device::Acquire(cDevice);
    device.SetUncapturedErrorCallback(errorCallback, nullptr);

    try {
        mainWebGPU();
    } catch (const std::exception& e) {
        std::cout << "Caught exception:" << std::endl;
        std::cerr << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Caught an unknown exception" << std::endl;
    }
}

#ifndef __EMSCRIPTEN__
void deviceLostCallback(WGPUDevice const * lostDevice, WGPUDeviceLostReason reason, char const * message, void * userdata) {
    switch (reason) {
        case WGPUDeviceLostReason_Unknown:
            std::cout << "instance lost for unknown reason" << std::endl;
            std::cout << "message: " << message << std::endl;
            break;
        case WGPUDeviceLostReason_Destroyed:
            std::cout << "instance destroyed" << std::endl;
            break;
        case WGPUDeviceLostReason_InstanceDropped:
            std::cout << "instance dropped" << std::endl;
            break;
        case WGPUDeviceLostReason_FailedCreation:
            std::cout << "instance failed creation" << std::endl;
            break;
        case WGPUDeviceLostReason_Force32:
            std::cout << "instance WGPUDeviceLostReason_Force32" << std::endl;
            std::cout << "message: " << message << std::endl;
            break;
        default:
            std::cout << "instance lost for reason not handled in the switch" << std::endl;
            std::cout << "reason: " << reason << std::endl;
            std::cout << "message: " << message << std::endl;
            break;
    }
}
#endif

void getAdapterCallback(WGPURequestAdapterStatus status, WGPUAdapter cAdapter, const char *message, void *userdata) {
    if (status != WGPURequestAdapterStatus_Success) {
        std::cout << "getAdapterCallback status not success" << std::endl;
        return;
    }
    adapter = wgpu::Adapter::Acquire(cAdapter);

    size_t numFeatures = adapter.EnumerateFeatures(nullptr);
    std::vector<wgpu::FeatureName> features(numFeatures);
    if (numFeatures > 0) {
        adapter.EnumerateFeatures(features.data());
    }

    std::vector<wgpu::FeatureName> requiredFeatures;

    bool canTimestamp =
            std::find(features.begin(), features.end(), wgpu::FeatureName::TimestampQuery) != features.end();
    if (canTimestamp) {
        requiredFeatures.push_back(wgpu::FeatureName::TimestampQuery);
    }

    wgpu::RequiredLimits limits = {};

    wgpu::DeviceDescriptor deviceDescriptor = {};
    deviceDescriptor.requiredFeatures = requiredFeatures.data();
    deviceDescriptor.requiredFeatureCount = requiredFeatures.size();
    deviceDescriptor.requiredLimits = &limits;

#ifndef __EMSCRIPTEN__
    wgpu::DeviceLostCallbackInfo deviceLostCallbackInfo = {};
    deviceLostCallbackInfo.callback = deviceLostCallback;
    deviceDescriptor.deviceLostCallbackInfo = deviceLostCallbackInfo;
#endif

    adapter.RequestDevice(&deviceDescriptor, getDeviceCallback, nullptr);
}

void getDevice() {
    wgpu::RequestAdapterOptions options = {};
    options.powerPreference = wgpu::PowerPreference::HighPerformance;
    instance.RequestAdapter(&options, getAdapterCallback, nullptr);
}

void webGPUTest() {
    std::cout << "webgpu test start" << std::endl;

    instance = wgpu::CreateInstance();
    getDevice();
}
