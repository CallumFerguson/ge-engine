#include "webgpu.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <lib_webgpu.h>

#include "gltfloader.hpp"

void webgpuTest() {
    std::cout << "webgpu test start" << std::endl;

    loadModelAndPrintVertexCount("assets/sphere.glb");

    if (navigator_gpu_available()) {
        std::cout << "WebGPU supported" << std::endl;
    } else {
        throw std::runtime_error("This browser does not support WebGPU");
    }

    WGpuRequestAdapterOptions options = {};
    options.powerPreference = WGPU_POWER_PREFERENCE_HIGH_PERFORMANCE;
    auto adapter = navigator_gpu_request_adapter_sync(&options);
    if (!wgpu_is_adapter(adapter)) {
        throw std::runtime_error("could not get adapter");
    }

    auto features = wgpu_adapter_get_features(adapter);

    int requiredFeatures = 0;

    bool canTimestamp = features & WGPU_FEATURE_TIMESTAMP_QUERY;
    if (canTimestamp) {
        requiredFeatures |= WGPU_FEATURE_TIMESTAMP_QUERY;
    }

    WGpuDeviceDescriptor deviceDesc = {};

    WGpuSupportedLimits requiredLimits = {};

    deviceDesc.requiredLimits = requiredLimits;
    deviceDesc.requiredFeatures = requiredFeatures;
    auto device = wgpu_adapter_request_device_sync(adapter, &deviceDesc);
    if (!wgpu_is_device(device)) {
        throw std::runtime_error("could not get device");
    }

    auto canvasContext = wgpu_canvas_get_webgpu_context("canvas");
    if (!wgpu_is_canvas_context(canvasContext)) {
        throw std::runtime_error("could not get context");
    }

    auto presentationFormat = navigator_gpu_get_preferred_canvas_format();

    WGpuCanvasConfiguration config = WGPU_CANVAS_CONFIGURATION_DEFAULT_INITIALIZER;
    config.device = device;
    config.format = presentationFormat;
    wgpu_canvas_context_configure(canvasContext, &config);

    auto colorAttachment = WGPU_RENDER_PASS_COLOR_ATTACHMENT_DEFAULT_INITIALIZER;
    WGpuColor clearColor{0, 0, 0, 1};
    colorAttachment.clearValue = clearColor;
    colorAttachment.loadOp = WGPU_LOAD_OP_CLEAR;
    colorAttachment.storeOp = WGPU_STORE_OP_STORE;
    colorAttachment.view = wgpu_texture_create_view(wgpu_canvas_context_get_current_texture(canvasContext));

    auto renderPassDescriptor = WGPU_RENDER_PASS_DESCRIPTOR_DEFAULT_INITIALIZER;
    renderPassDescriptor.numColorAttachments = 1;
    renderPassDescriptor.colorAttachments = &colorAttachment;

    auto commandEncoder = wgpu_device_create_command_encoder_simple(device);

    auto renderPassEncoder = wgpu_command_encoder_begin_render_pass(commandEncoder, &renderPassDescriptor);

    std::ifstream shaderFile("shaders/fullscreen_color.wgsl", std::ios::binary);
    if(!shaderFile) {
        throw std::runtime_error("Could not open shader file");
    }
    std::stringstream shaderBuffer;
    shaderBuffer << shaderFile.rdbuf();

    WGpuShaderModuleDescriptor shaderModuleDescriptor = {};
    shaderModuleDescriptor.code = shaderBuffer.str().c_str();
    auto shaderModule = wgpu_device_create_shader_module(device, &shaderModuleDescriptor);

    auto pipelineLayout = wgpu_device_create_pipeline_layout(device, nullptr, 0);

    auto pipelineDescriptor = WGPU_RENDER_PIPELINE_DESCRIPTOR_DEFAULT_INITIALIZER;
    pipelineDescriptor.layout = pipelineLayout;

    pipelineDescriptor.vertex.module = shaderModule;
    pipelineDescriptor.vertex.entryPoint = "vert";
    pipelineDescriptor.vertex.numBuffers = 0;

    auto target = WGPU_COLOR_TARGET_STATE_DEFAULT_INITIALIZER;
    target.format = presentationFormat;

    pipelineDescriptor.fragment.module = shaderModule;
    pipelineDescriptor.fragment.entryPoint = "frag";
    pipelineDescriptor.fragment.numTargets = 1;
    pipelineDescriptor.fragment.targets = &target;

    pipelineDescriptor.primitive.topology = WGPU_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipelineDescriptor.primitive.cullMode = WGPU_CULL_MODE_BACK;

    pipelineDescriptor.multisample.count = 1;

    auto pipeline = wgpu_device_create_render_pipeline(device, &pipelineDescriptor);

    wgpu_render_pass_encoder_set_pipeline(renderPassEncoder, pipeline);
    wgpu_render_pass_encoder_draw(renderPassEncoder, 3, 1, 0, 0);

    wgpu_render_pass_encoder_end(renderPassEncoder);

    auto queue = wgpu_device_get_queue(device);
    wgpu_queue_submit_one_and_destroy(queue, wgpu_command_encoder_finish(commandEncoder));

}