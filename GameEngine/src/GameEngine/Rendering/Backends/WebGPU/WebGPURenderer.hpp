#pragma once

#include <functional>
#include <webgpu/webgpu_cpp.h>

namespace GameEngine {

class Window;

class WebGPURenderer {
public:
    static void init(Window *window);

    static void configureSurface();

    static void present();

    static void startFrame();

    static void startMainRenderPass();

    static void endMainRenderPass();

    static void endFrame();

    static wgpu::TextureFormat mainSurfacePreferredFormat();

    static wgpu::Device &device();

    static wgpu::RenderPassEncoder &renderPassEncoder();

    static wgpu::CommandEncoder &commandEncoder();

    static const wgpu::Buffer &cameraDataBuffer();

    static const wgpu::BindGroupLayout &cameraDataBindGroupLayout();

    static const wgpu::BindGroup &cameraDataBindGroup();

    static void updateCameraDataBuffer(const glm::mat4 &view, const glm::mat4 &projection);

private:
    static void getAdapter();

    static void getAdapterCallback(
            WGPURequestAdapterStatus status, WGPUAdapter cAdapter, const char *message, void *userdata);

    static void getDeviceCallback(
            WGPURequestDeviceStatus status, WGPUDevice cDevice, const char *message, void *userdata);

    static void finishInit();

    static void createSurface();

#ifndef __EMSCRIPTEN__

    static void deviceLostCallback(
            WGPUDevice const *lostDevice, WGPUDeviceLostReason reason, char const *message, void *userdata);

#endif

    static void errorCallback(WGPUErrorType type, const char *message, void *userdata);

    static void setUpCameraBindGroup();
};

}
