#pragma once

#include <functional>
#include <webgpu/webgpu_cpp.h>
#include "../../../Scene/Entity.hpp"

namespace GameEngine {

struct WebGPUPBRRendererDataComponent {
    wgpu::Buffer objectDataBuffer;
    wgpu::BindGroup objectDataBindGroup;

    WebGPUPBRRendererDataComponent() = delete;

    explicit WebGPUPBRRendererDataComponent(int materialHandle);

    [[nodiscard]] const char *objectName() const {
        return "WebGPUPBRRendererDataComponent";
    }
};

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

    static void updateCameraDataBuffer(const glm::mat4 &view, const glm::mat4 &projection);

    static void renderMesh(Entity &entity, const PBRRendererComponent &renderer, const WebGPUPBRRendererDataComponent &rendererData);

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

    static void setUpCameraBuffer();

    static wgpu::RenderPipeline createPBRRenderPipeline(const wgpu::ShaderModule &shaderModule);
};

}
