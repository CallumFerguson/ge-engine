#include "WebGPURenderer.hpp"

#include <iostream>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_wgpu.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../../../Core/Window.hpp"
#include "../../../Core/Exit.hpp"
#include "../../../Utility/utility.hpp"

#ifdef __EMSCRIPTEN__

#include <emscripten.h>

#else

#include <webgpu/webgpu_glfw.h>

#endif

namespace GameEngine {

static bool s_initFinished;
static bool s_initializedSuccessfully;

static Window *s_window;

static wgpu::Instance s_instance;
static wgpu::Adapter s_adapter;
static wgpu::Device s_device;
static wgpu::Surface s_surface;
#ifdef __EMSCRIPTEN__
static wgpu::SwapChain s_swapChain;
#endif

static wgpu::TextureFormat s_mainSurfacePreferredFormat;

static wgpu::RenderPassColorAttachment s_colorAttachment;
static wgpu::RenderPassDescriptor s_renderPassDescriptor;

static wgpu::RenderPassEncoder s_renderPassEncoder;
static wgpu::CommandEncoder s_commandEncoder;

static wgpu::Buffer s_cameraDataBuffer;
static wgpu::BindGroupLayout s_cameraDataBindGroupLayout;
static wgpu::BindGroup s_cameraDataBindGroup;

void WebGPURenderer::init(Window *window) {
    s_window = window;
    s_initFinished = false;
    s_instance = wgpu::CreateInstance();
    getAdapter();

#ifdef __EMSCRIPTEN__
    while (!s_initFinished) {
        emscripten_sleep(10);
    }
#endif

    if (!s_initializedSuccessfully) {
        exitApp("failed to initialize WebGPURenderer");
    }
}

void WebGPURenderer::getAdapter() {
    wgpu::RequestAdapterOptions options = {};
    options.powerPreference = wgpu::PowerPreference::HighPerformance;
    s_instance.RequestAdapter(&options, getAdapterCallback, nullptr);
}

void WebGPURenderer::getAdapterCallback(
        WGPURequestAdapterStatus status, WGPUAdapter adapterHandle, const char *message, void *userdata) {
    if (status != WGPURequestAdapterStatus_Success) {
        std::cout << "getAdapterCallback status not success" << std::endl;
        s_initializedSuccessfully = false;
        s_initFinished = true;
        return;
    }
    s_adapter = wgpu::Adapter::Acquire(adapterHandle);

    size_t numFeatures = s_adapter.EnumerateFeatures(nullptr);
    std::vector<wgpu::FeatureName> features(numFeatures);
    if (numFeatures > 0) {
        s_adapter.EnumerateFeatures(features.data());
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

    s_adapter.RequestDevice(&deviceDescriptor, getDeviceCallback, nullptr);
}

void WebGPURenderer::getDeviceCallback(
        WGPURequestDeviceStatus status, WGPUDevice cDevice, const char *message, void *userdata) {
    s_device = wgpu::Device::Acquire(cDevice);
    s_device.SetUncapturedErrorCallback(errorCallback, nullptr);

    finishInit();

    s_initializedSuccessfully = true;
    s_initFinished = true;
}

void WebGPURenderer::finishInit() {
    createSurface();
    s_mainSurfacePreferredFormat = s_surface.GetPreferredFormat(s_adapter);
    configureSurface();

    ImGui_ImplWGPU_InitInfo init_info;
    init_info.Device = s_device.Get();
    init_info.NumFramesInFlight = 3;
    init_info.RenderTargetFormat = (WGPUTextureFormat) s_mainSurfacePreferredFormat;
    init_info.DepthStencilFormat = WGPUTextureFormat_Undefined;
    ImGui_ImplWGPU_Init(&init_info);

    s_colorAttachment = {};
    s_colorAttachment.loadOp = wgpu::LoadOp::Clear;
    s_colorAttachment.storeOp = wgpu::StoreOp::Store;
    s_colorAttachment.clearValue = wgpu::Color{0, 0, 0, 1};

    s_renderPassDescriptor = {};
    s_renderPassDescriptor.colorAttachmentCount = 1;
    s_renderPassDescriptor.colorAttachments = &s_colorAttachment;

    // camera

    setUpCameraBindGroup();
}

void WebGPURenderer::createSurface() {
#ifdef __EMSCRIPTEN__
    wgpu::SurfaceDescriptorFromCanvasHTMLSelector canvasDescriptor = {};
    canvasDescriptor.selector = "#canvas";

    wgpu::SurfaceDescriptor surfaceDescriptor = {};
    surfaceDescriptor.nextInChain = &canvasDescriptor;

    s_surface = s_instance.CreateSurface(&surfaceDescriptor);
#else
    s_surface = wgpu::glfw::CreateSurfaceForWindow(s_instance, s_window->m_glfwWindow);
#endif
}

#ifndef __EMSCRIPTEN__

void WebGPURenderer::deviceLostCallback(
        WGPUDevice const *lostDevice, WGPUDeviceLostReason reason, char const *message, void *userdata) {
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

void WebGPURenderer::errorCallback(WGPUErrorType type, const char *message, void *userdata) {
    std::cout << "Error: " << type << " - message: " << message << std::endl;
    s_initializedSuccessfully = false;
    s_initFinished = true;
}

void WebGPURenderer::configureSurface() {
    // TODO: use configure api for both https://github.com/emscripten-core/emscripten/pull/21939
#ifdef __EMSCRIPTEN__
    wgpu::SwapChainDescriptor swapChainDescriptor = {};
    swapChainDescriptor.usage = wgpu::TextureUsage::RenderAttachment;
    swapChainDescriptor.format = s_mainSurfacePreferredFormat;
    swapChainDescriptor.width = s_window->renderSurfaceWidth();
    swapChainDescriptor.height = s_window->renderSurfaceHeight();
    swapChainDescriptor.presentMode = wgpu::PresentMode::Fifo;
    s_swapChain = s_device.CreateSwapChain(s_surface, &swapChainDescriptor);
#else
    wgpu::SurfaceConfiguration surfaceConfiguration = {};
    surfaceConfiguration.device = s_device;
    surfaceConfiguration.format = s_mainSurfacePreferredFormat;
    surfaceConfiguration.width = s_window->renderSurfaceWidth();
    surfaceConfiguration.height = s_window->renderSurfaceHeight();
    s_surface.Configure(&surfaceConfiguration);
#endif
}

void WebGPURenderer::present() {
#ifndef __EMSCRIPTEN__
    s_surface.Present();
    s_instance.ProcessEvents();
#endif
}

void WebGPURenderer::startFrame() {
    ImGui_ImplWGPU_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

#ifndef __EMSCRIPTEN__
    // Tick needs to be called in Dawn to display validation errors
    // https://github.com/ocornut/imgui/blob/master/examples/example_glfw_wgpu/main.cpp
    s_device.Tick();
#endif

#ifdef __EMSCRIPTEN__
    auto currentSurfaceTextureView = s_swapChain.GetCurrentTextureView();
#else
    wgpu::SurfaceTexture currentSurfaceTexture;
    s_surface.GetCurrentTexture(&currentSurfaceTexture);
    auto currentSurfaceTextureView = currentSurfaceTexture.texture.CreateView();
#endif

    s_colorAttachment.view = currentSurfaceTextureView;

    s_commandEncoder = s_device.CreateCommandEncoder();
}

void WebGPURenderer::startMainRenderPass() {
    s_renderPassEncoder = s_commandEncoder.BeginRenderPass(&s_renderPassDescriptor);
}

void WebGPURenderer::endMainRenderPass() {
    ImGui::Render();
    ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), s_renderPassEncoder.Get());

    s_renderPassEncoder.End();
}

void WebGPURenderer::endFrame() {
    auto commandBuffer = s_commandEncoder.Finish();
    s_device.GetQueue().Submit(1, &commandBuffer);
}

wgpu::TextureFormat WebGPURenderer::mainSurfacePreferredFormat() {
    return s_mainSurfacePreferredFormat;
}

wgpu::Device &WebGPURenderer::device() {
    return s_device;
}

wgpu::RenderPassEncoder &WebGPURenderer::renderPassEncoder() {
    return s_renderPassEncoder;
}

wgpu::CommandEncoder &WebGPURenderer::commandEncoder() {
    return s_commandEncoder;
}

const wgpu::Buffer &WebGPURenderer::cameraDataBuffer() {
    return s_cameraDataBuffer;
}

const wgpu::BindGroupLayout &WebGPURenderer::cameraDataBindGroupLayout() {
    return s_cameraDataBindGroupLayout;
}

const wgpu::BindGroup &WebGPURenderer::cameraDataBindGroup() {
    return s_cameraDataBindGroup;
}

void WebGPURenderer::setUpCameraBindGroup() {
    wgpu::BufferBindingLayout cameraDataBindGroupLayoutEntry0BufferBindingLayout = {};
    cameraDataBindGroupLayoutEntry0BufferBindingLayout.type = wgpu::BufferBindingType::Uniform;

    wgpu::BindGroupLayoutEntry cameraDataBindGroupLayoutEntry0 = {};
    cameraDataBindGroupLayoutEntry0.binding = 0;
    cameraDataBindGroupLayoutEntry0.visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
    cameraDataBindGroupLayoutEntry0.buffer = cameraDataBindGroupLayoutEntry0BufferBindingLayout;

    wgpu::BindGroupLayoutDescriptor cameraDataBindGroupLayoutDescriptor = {};
    cameraDataBindGroupLayoutDescriptor.entryCount = 1;
    cameraDataBindGroupLayoutDescriptor.entries = &cameraDataBindGroupLayoutEntry0;
    s_cameraDataBindGroupLayout = s_device.CreateBindGroupLayout(&cameraDataBindGroupLayoutDescriptor);

    wgpu::BufferDescriptor bufferDescriptor = {};
    bufferDescriptor.size = 64 * 2;
    bufferDescriptor.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
    bufferDescriptor.mappedAtCreation = false;
    s_cameraDataBuffer = s_device.CreateBuffer(&bufferDescriptor);

    wgpu::BindGroupEntry cameraDataBindGroupEntry0 = {};
    cameraDataBindGroupEntry0.binding = 0;
    cameraDataBindGroupEntry0.buffer = s_cameraDataBuffer;

    wgpu::BindGroupDescriptor cameraDataBindGroupDescriptor = {};
    cameraDataBindGroupDescriptor.layout = s_cameraDataBindGroupLayout;
    cameraDataBindGroupDescriptor.entryCount = 1;
    cameraDataBindGroupDescriptor.entries = &cameraDataBindGroupEntry0;

    s_cameraDataBindGroup = s_device.CreateBindGroup(&cameraDataBindGroupDescriptor);
}

void WebGPURenderer::updateCameraDataBuffer(const glm::mat4 &view, const glm::mat4 &projection) {
    uint8_t data[128];
    std::memcpy(data, glm::value_ptr(view), 64);
    std::memcpy(data + 64, glm::value_ptr(projection), 64);
    device().GetQueue().WriteBuffer(s_cameraDataBuffer, 0, data, 128);
}

void WebGPURenderer::renderMesh(Entity &entity, const MeshRendererComponent &meshRenderer) {
    std::cout << "render " << (int) entity.enttHandle() << std::endl;
}

}
