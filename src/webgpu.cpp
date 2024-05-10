#include "webgpu.hpp"

#include <iostream>
#include <emscripten.h>
#include <lib_webgpu.h>
#include <assert.h>

#include "gltfloader.hpp"

//void ObtainedWebGpuDevice(WGpuDevice result, void *userData) {
//    device = result;
//
//    if (!device) {
//        std::cout << "could not get device" << std::endl;
//    }
//
//    std::cout << "got device" << std::endl;
//    std::cout << device << std::endl;
//}
//
//void ObtainedWebGpuAdapter(WGpuAdapter result, void *userData) {
//    adapter = result;
//
//    if (!adapter) {
//        std::cout << "could not get adapter" << std::endl;
//    }
//
//    std::cout << "got adapter" << std::endl;
//    std::cout << adapter << std::endl;
//
//    WGpuDeviceDescriptor deviceDesc = {};
//
//    WGpuSupportedLimits requiredLimits = {};
//
//    deviceDesc.requiredLimits = requiredLimits;
//    wgpu_adapter_request_device_async(adapter, &deviceDesc, ObtainedWebGpuDevice, nullptr);
//}

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

    auto features = wgpu_adapter_or_device_get_features(adapter);

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
}