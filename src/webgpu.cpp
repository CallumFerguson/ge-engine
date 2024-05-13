#include "webgpu.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <webgpu/webgpu_cpp.h>
#include <emscripten.h>

#include "gltfloader.hpp"

wgpu::Instance instance;
wgpu::Adapter adapter;
wgpu::Device device;
wgpu::SwapChain swapChain;
wgpu::RenderPipeline pipeline;
wgpu::TextureFormat presentationFormat;

void resizeCanvas() {
    wgpu::SupportedLimits supportedLimits = {};
    device.GetLimits(&supportedLimits);

    // @formatter:off
    EM_ASM({
               const canvas = Module.canvas;
               const width = Math.max(1, Math.min($0, canvas.clientWidth));
               const height = Math.max(1, Math.min($0, canvas.clientHeight));

               const needResize = width !== canvas.width || height !== canvas.height;
               if (needResize) {
                   canvas.width = width;
                   canvas.height = height;
               }
           }, supportedLimits.limits.maxTextureDimension2D);
    // @formatter:on
}

uint32_t getCanvasWidth() {
    return EM_ASM_INT(
            if (Module.canvas) {
                return canvas.width;
            }
            return 0;
    );
}

uint32_t getCanvasHeight() {
    return EM_ASM_INT(
            if (Module.canvas) {
                return canvas.height;
            }
            return 0;
    );
}

void CreateRenderPipeline() {
    std::ifstream shaderFile("shaders/basic_triangle.wgsl", std::ios::binary);
    if (!shaderFile) {
        throw std::runtime_error("Could not open shader file");
    }
    std::stringstream shaderBuffer;
    shaderBuffer << shaderFile.rdbuf();
    std::string shaderString = shaderBuffer.str();

    wgpu::ShaderModuleWGSLDescriptor wgslDesc{};
    wgslDesc.code = shaderString.c_str();

    wgpu::ShaderModuleDescriptor shaderModuleDescriptor{
            .nextInChain = &wgslDesc};
    wgpu::ShaderModule shaderModule =
            device.CreateShaderModule(&shaderModuleDescriptor);

    wgpu::ColorTargetState colorTargetState{
            .format = presentationFormat};

    wgpu::FragmentState fragmentState{.module = shaderModule,
    .targetCount = 1,
    .targets = &colorTargetState};

    wgpu::RenderPipelineDescriptor descriptor{
            .vertex = {.module = shaderModule},
            .fragment = &fragmentState};
    pipeline = device.CreateRenderPipeline(&descriptor);
}

void Render() {
    wgpu::RenderPassColorAttachment attachment{
            .view = swapChain.GetCurrentTextureView(),
            .loadOp = wgpu::LoadOp::Clear,
            .storeOp = wgpu::StoreOp::Store};
    attachment.clearValue = wgpu::Color{0.25, 0, 0, 1};

    wgpu::RenderPassDescriptor renderpass{.colorAttachmentCount = 1,
            .colorAttachments = &attachment};

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderpass);
    pass.SetPipeline(pipeline);
    pass.Draw(3);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    device.GetQueue().Submit(1, &commands);
}

void mainWebGPU() {
    wgpu::SupportedLimits limits;
    device.GetLimits(&limits);
    std::cout << "Maximum storage buffer size: " << limits.limits.maxStorageBufferBindingSize << std::endl;

    wgpu::SurfaceDescriptorFromCanvasHTMLSelector canvasDescriptor = {};
    canvasDescriptor.selector = "#canvas";

    wgpu::SurfaceDescriptor surfaceDescriptor = {};
    surfaceDescriptor.nextInChain = &canvasDescriptor;

    auto surface = instance.CreateSurface(&surfaceDescriptor);

    presentationFormat = surface.GetPreferredFormat(adapter);

    resizeCanvas();

    wgpu::SwapChainDescriptor swapChainDescriptor = {};
    swapChainDescriptor.usage = wgpu::TextureUsage::RenderAttachment;
    swapChainDescriptor.format = presentationFormat;
    swapChainDescriptor.width = getCanvasWidth();
    swapChainDescriptor.height = getCanvasHeight();
    swapChainDescriptor.presentMode = wgpu::PresentMode::Fifo;
    swapChain = device.CreateSwapChain(surface, &swapChainDescriptor);

    CreateRenderPipeline();
    Render();
}

void errorCallback(WGPUErrorType type, const char *message, void *userdata) {
    std::ostringstream oss;
    oss << "Error: " << type << " - message: " << message;
    throw std::runtime_error(oss.str());
}

void getDeviceCallback(WGPURequestDeviceStatus status, WGPUDevice cDevice, const char *message, void *userdata) {
    device = wgpu::Device::Acquire(cDevice);
    device.SetUncapturedErrorCallback(errorCallback, nullptr);

    mainWebGPU();
}

void getAdapterCallback(WGPURequestAdapterStatus status, WGPUAdapter cAdapter, const char *message, void *userdata) {
    if (status != WGPURequestAdapterStatus_Success) {
        std::cout << status << std::endl;

        throw std::runtime_error("getAdapterCallback status not success");
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

    adapter.RequestDevice(&deviceDescriptor, getDeviceCallback, nullptr);
}

void getDevice() {
    wgpu::RequestAdapterOptions options = {};
    options.powerPreference = wgpu::PowerPreference::HighPerformance;
    instance.RequestAdapter(&options, getAdapterCallback, nullptr);
}

void webgpuTest() {
    std::cout << "webgpu test start" << std::endl;

    loadModelAndPrintVertexCount("assets/sphere.glb");

    instance = wgpu::CreateInstance();

    getDevice();
}

//void webgpuTest() {
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
