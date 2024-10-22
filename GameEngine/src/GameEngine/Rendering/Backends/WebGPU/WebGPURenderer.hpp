#pragma once

#include <functional>
#include <webgpu/webgpu_cpp.h>
#include "../../../Scene/Entity.hpp"
#include "../../Mesh.hpp"
#include "../../Material.hpp"

#define PBR_SHADER_UUID "7aa2b713-86dd-4c48-a9ea-9af110d116ee"
#define BRDF_UUID "7062cece-527b-4945-b26f-a6302c632c9c"
#define EQUIRECTANGULAR_SKYBOX_SHADER_UUID "20b9adeb-c3d4-4bd4-8e0c-18e0b3238af9"
#define SKYBOX_SHADER_UUID "8c9465d7-3898-4516-b48e-24ef8a1b3296"
#define CALCULATE_IRRADIANCE_SHADER_UUID "cb03c587-a565-4da6-8325-227596aa4dcd"
#define CALCULATE_PRE_FILTER_SHADER_UUID "c14e9db1-a5e9-46d9-99d2-498a8869dd38"

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

struct MeshRenderInfo {
    Mesh &mesh;
    Material &material;
    const wgpu::BindGroup &objectDataBindGroup;
};

class Window;

class WebGPURenderer {
public:
    static const uint32_t multisampleCount = 4;

    static void init(Window *window, std::vector<wgpu::FeatureName> requiredFeatures = {});

    static void configureSurface();

    static void present();

    static void startFrame();

    static void startMainRenderPass();

    static void endMainRenderPass();

    static void endFrame();

    static wgpu::TextureFormat mainSurfacePreferredFormat();

    static wgpu::Device &device();

    static wgpu::Instance &instance();

    static wgpu::RenderPassEncoder &renderPassEncoder();

    static wgpu::CommandEncoder &commandEncoder();

    static const wgpu::Buffer &cameraDataBuffer();

    static void updateCameraDataBuffer(Entity &entity, TransformComponent &transform, CameraComponent &camera);

    static void submitMeshToRenderer(Entity &entity, const PBRRendererComponent &renderer, const WebGPUPBRRendererDataComponent &rendererData);

    static void drawMainRenderPass();

    static wgpu::Sampler &basicSampler();

    static wgpu::RenderPipeline createBasicPipeline(const wgpu::ShaderModule &shaderModule, bool renderToScreen, bool depthWrite, wgpu::TextureFormat textureFormat = wgpu::TextureFormat::Undefined);

    static wgpu::QueueWorkDoneStatus waitForDeviceIdle();

    static wgpu::BindGroupLayout &pbrEnvironmentMapBindGroupLayout();

    static wgpu::BindGroupLayout &cameraDataBindGroupLayout();

    static wgpu::BindGroupLayout &pbrMaterialBindGroupLayout();

    static wgpu::BindGroupLayout &pbrObjectDataBindGroupLayout();

    static wgpu::BindGroup &cameraDataBindGroup();

    static void setEnvironmentMap(int environmentMapHandle);

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

    static wgpu::RenderPipeline createPBRRenderPipeline(const wgpu::ShaderModule &shaderModule, bool depthWrite);

    static wgpu::RenderPipeline createSkyboxRenderPipeline(const wgpu::ShaderModule &shaderModule, bool depthWrite);

    static void drawMesh(MeshRenderInfo &meshRenderInfo);

    static void drawOpaqueMeshes();

    static void drawSkybox();

    static void drawTransparentMeshes();
};

}
