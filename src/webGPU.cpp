#include "webGPU.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <webgpu/webgpu_cpp.h>

#ifdef __EMSCRIPTEN__

#include "webGPUEmscripten.hpp"
#include <emscripten.h>

#else

#include "webGPUDawn.hpp"
#include <webgpu/webgpu_glfw.h>
#include <GLFW/glfw3.h>

#endif

wgpu::Instance instance;
wgpu::Adapter adapter;
wgpu::Device device;
wgpu::Surface surface;
wgpu::RenderPipeline pipeline;

wgpu::RenderPassColorAttachment colorAttachment;
wgpu::RenderPassDescriptor renderPassDescriptor;

#ifdef __EMSCRIPTEN__
wgpu::SwapChain swapChain;
#endif

void render() {
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
    renderPassEncoder.End();

    auto commandBuffer = commandEncoder.Finish();
    device.GetQueue().Submit(1, &commandBuffer);
}

void mainWebGPU() {
    wgpu::SupportedLimits limits;
    device.GetLimits(&limits);
    std::cout << "Maximum storage buffer size: " << limits.limits.maxStorageBufferBindingSize << std::endl;

#ifdef __EMSCRIPTEN__
    wgpu::SurfaceDescriptorFromCanvasHTMLSelector canvasDescriptor = {};
    canvasDescriptor.selector = "#canvas";

    wgpu::SurfaceDescriptor surfaceDescriptor = {};
    surfaceDescriptor.nextInChain = &canvasDescriptor;

    surface = instance.CreateSurface(&surfaceDescriptor);

    resizeCanvas(device);
    uint32_t width = getCanvasWidth();
    uint32_t height = getCanvasHeight();
#else
    if (!glfwInit()) {
        std::cout << "could not glfwInit" << std::endl;
        return;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    auto window = glfwCreateWindow(512, 512, "WebGPU window", nullptr, nullptr);

    surface = wgpu::glfw::CreateSurfaceForWindow(instance, window);

    uint32_t width = 512;
    uint32_t height = 512;
#endif

    auto presentationFormat = surface.GetPreferredFormat(adapter);

#ifdef __EMSCRIPTEN__
    wgpu::SwapChainDescriptor swapChainDescriptor = {};
    swapChainDescriptor.usage = wgpu::TextureUsage::RenderAttachment;
    swapChainDescriptor.format = presentationFormat;
    swapChainDescriptor.width = width;
    swapChainDescriptor.height = width;
    swapChainDescriptor.presentMode = wgpu::PresentMode::Fifo;
    swapChain = device.CreateSwapChain(surface, &swapChainDescriptor);
#else
    wgpu::SurfaceConfiguration surfaceConfiguration = {};
    surfaceConfiguration.device = device;
    surfaceConfiguration.format = presentationFormat;
    surfaceConfiguration.width = width;
    surfaceConfiguration.height = height;
    surface.Configure(&surfaceConfiguration);
#endif

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
    emscripten_set_main_loop(render, 0, false); // TODO: try simulate loop, also animation loop variables? or just use c++ stuff to get current time
#else
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        render();

        surface.Present();
        instance.ProcessEvents();
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

//void webGPUTest() {
//    std::cout << "webgpu test start" << std::endl;
//
//    loadModelAndPrintVertexCount("assets/sphere.glb");
//
//    if (navigator_gpu_available()) {
//        std::cout << "WebGPU supported" << std::endl;
//    } else {
//        throw std::runtime_error("This browser does not support WebGPU");
//    }
//
//    WGpuRequestAdapterOptions options = {};
//    options.powerPreference = WGPU_POWER_PREFERENCE_HIGH_PERFORMANCE;
//    auto adapter = navigator_gpu_request_adapter_sync(&options);
//    if (!wgpu_is_adapter(adapter)) {
//        throw std::runtime_error("could not get adapter");
//    }
//
//    auto features = wgpu_adapter_get_features(adapter);
//
//    int requiredFeatures = 0;
//
//    bool canTimestamp = features & WGPU_FEATURE_TIMESTAMP_QUERY;
//    if (canTimestamp) {
//        requiredFeatures |= WGPU_FEATURE_TIMESTAMP_QUERY;
//    }
//
//    WGpuDeviceDescriptor deviceDesc = {};
//
//    WGpuSupportedLimits requiredLimits = {};
//
//    deviceDesc.requiredLimits = requiredLimits;
//    deviceDesc.requiredFeatures = requiredFeatures;
//    auto device = wgpu_adapter_request_device_sync(adapter, &deviceDesc);
//    if (!wgpu_is_device(device)) {
//        throw std::runtime_error("could not get device");
//    }
//
//    auto canvasContext = wgpu_canvas_get_webgpu_context("canvas");
//    if (!wgpu_is_canvas_context(canvasContext)) {
//        throw std::runtime_error("could not get context");
//    }
//
//    auto presentationFormat = navigator_gpu_get_preferred_canvas_format();
//
//    WGpuCanvasConfiguration config = WGPU_CANVAS_CONFIGURATION_DEFAULT_INITIALIZER;
//    config.device = device;
//    config.format = presentationFormat;
//    wgpu_canvas_context_configure(canvasContext, &config);
//
//    auto colorAttachment = WGPU_RENDER_PASS_COLOR_ATTACHMENT_DEFAULT_INITIALIZER;
//    WGpuColor clearColor{0, 0, 0, 1};
//    colorAttachment.clearValue = clearColor;
//    colorAttachment.loadOp = WGPU_LOAD_OP_CLEAR;
//    colorAttachment.storeOp = WGPU_STORE_OP_STORE;
//    colorAttachment.view = wgpu_texture_create_view(wgpu_canvas_context_get_current_texture(canvasContext));
//
//    auto renderPassDescriptor = WGPU_RENDER_PASS_DESCRIPTOR_DEFAULT_INITIALIZER;
//    renderPassDescriptor.numColorAttachments = 1;
//    renderPassDescriptor.colorAttachments = &colorAttachment;
//
//    auto commandEncoder = wgpu_device_create_command_encoder_simple(device);
//
//    auto renderPassEncoder = wgpu_command_encoder_begin_render_pass(commandEncoder, &renderPassDescriptor);
//
//    std::ifstream shaderFile("shaders/fullscreen_color.wgsl", std::ios::binary);
//    if(!shaderFile) {
//        throw std::runtime_error("Could not open shader file");
//    }
//    std::stringstream shaderBuffer;
//    shaderBuffer << shaderFile.rdbuf();
//
//    std::string shaderString = shaderBuffer.str();
//
//    WGpuShaderModuleDescriptor shaderModuleDescriptor = {};
//    shaderModuleDescriptor.code = shaderString.c_str();
//    auto shaderModule = wgpu_device_create_shader_module(device, &shaderModuleDescriptor);
//
//    auto pipelineLayout = wgpu_device_create_pipeline_layout(device, nullptr, 0);
//
//    auto pipelineDescriptor = WGPU_RENDER_PIPELINE_DESCRIPTOR_DEFAULT_INITIALIZER;
//    pipelineDescriptor.layout = pipelineLayout;
//
//    pipelineDescriptor.vertex.module = shaderModule;
//    pipelineDescriptor.vertex.entryPoint = "vert";
//    pipelineDescriptor.vertex.numBuffers = 0;
//
//    auto target = WGPU_COLOR_TARGET_STATE_DEFAULT_INITIALIZER;
//    target.format = presentationFormat;
//
//    pipelineDescriptor.fragment.module = shaderModule;
//    pipelineDescriptor.fragment.entryPoint = "frag";
//    pipelineDescriptor.fragment.numTargets = 1;
//    pipelineDescriptor.fragment.targets = &target;
//
//    pipelineDescriptor.primitive.topology = WGPU_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
//    pipelineDescriptor.primitive.cullMode = WGPU_CULL_MODE_BACK;
//
//    pipelineDescriptor.multisample.count = 1;
//
//    auto pipeline = wgpu_device_create_render_pipeline(device, &pipelineDescriptor);
//
//    wgpu_render_pass_encoder_set_pipeline(renderPassEncoder, pipeline);
//    wgpu_render_pass_encoder_draw(renderPassEncoder, 3, 1, 0, 0);
//
//    wgpu_render_pass_encoder_end(renderPassEncoder);
//
//    auto queue = wgpu_device_get_queue(device);
//    wgpu_queue_submit_one_and_destroy(queue, wgpu_command_encoder_finish(commandEncoder));
//}
