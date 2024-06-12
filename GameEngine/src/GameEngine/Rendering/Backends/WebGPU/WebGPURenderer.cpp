#include "WebGPURenderer.hpp"

#include <array>
#include <iostream>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_wgpu.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../../../Core/Window.hpp"
#include "../../../Core/Exit.hpp"
#include "../../../Utility/utility.hpp"
#include "../../../Assets/AssetManager.hpp"

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

static wgpu::TextureFormat s_mainSurfacePreferredFormat;

const uint32_t multisampleCount = 4;

static wgpu::RenderPassColorAttachment s_colorAttachment;
static wgpu::RenderPassDepthStencilAttachment s_depthAttachment;
static wgpu::RenderPassDescriptor s_renderPassDescriptor;

static wgpu::RenderPassEncoder s_renderPassEncoder;
static wgpu::CommandEncoder s_commandEncoder;

static wgpu::Buffer s_cameraDataBuffer;
static wgpu::BindGroup s_cameraDataBindGroup;

static wgpu::BindGroup s_materialBindGroup;

static wgpu::RenderPipeline s_pbrRenderPipeline;

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

    ImGui_ImplWGPU_InitInfo init_info;
    init_info.Device = s_device.Get();
    init_info.NumFramesInFlight = 3;
    init_info.RenderTargetFormat = static_cast<WGPUTextureFormat>(s_mainSurfacePreferredFormat);
    init_info.DepthStencilFormat = static_cast<WGPUTextureFormat>(wgpu::TextureFormat::Depth24Plus);
    init_info.PipelineMultisampleState.count = multisampleCount;
    ImGui_ImplWGPU_Init(&init_info);

    s_colorAttachment = {};
    s_colorAttachment.loadOp = wgpu::LoadOp::Clear;
    s_colorAttachment.storeOp = wgpu::StoreOp::Store;
    s_colorAttachment.clearValue = wgpu::Color{0, 0, 0, 1};

    s_depthAttachment = {};
    s_depthAttachment.depthClearValue = 1.0f;
    s_depthAttachment.depthLoadOp = wgpu::LoadOp::Clear;
    s_depthAttachment.depthStoreOp = wgpu::StoreOp::Store;

    s_renderPassDescriptor = {};
    s_renderPassDescriptor.colorAttachmentCount = 1;
    s_renderPassDescriptor.colorAttachments = &s_colorAttachment;
    s_renderPassDescriptor.depthStencilAttachment = &s_depthAttachment;

    configureSurface();

    setUpPBRRenderPipeline();
    setUpCameraBuffer();

    wgpu::SamplerDescriptor samplerDescriptor;
    samplerDescriptor.magFilter = wgpu::FilterMode::Linear;
    samplerDescriptor.minFilter = wgpu::FilterMode::Linear;
    samplerDescriptor.mipmapFilter = wgpu::MipmapFilterMode::Linear;
    auto sampler = s_device.CreateSampler(&samplerDescriptor);

    wgpu::BindGroupEntry bindGroupDescriptorEntry0 = {};
    bindGroupDescriptorEntry0.binding = 0;
    bindGroupDescriptorEntry0.sampler = sampler;

    Texture texture("assets/packaged/FlightHelmet/FlightHelmet_Materials_LeatherPartsMat_BaseColor.getexture");

    wgpu::BindGroupEntry bindGroupDescriptorEntry1 = {};
    bindGroupDescriptorEntry1.binding = 1;
    bindGroupDescriptorEntry1.textureView = texture.texture().CreateView();

    std::array<wgpu::BindGroupEntry, 2> bindGroupEntries = {bindGroupDescriptorEntry0, bindGroupDescriptorEntry1};

    wgpu::BindGroupDescriptor bindGroupDescriptor = {};
    bindGroupDescriptor.layout = s_pbrRenderPipeline.GetBindGroupLayout(1);
    bindGroupDescriptor.entryCount = bindGroupEntries.size();
    bindGroupDescriptor.entries = bindGroupEntries.data();

    s_materialBindGroup = s_device.CreateBindGroup(&bindGroupDescriptor);
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
    wgpu::SurfaceConfiguration surfaceConfiguration = {};
    surfaceConfiguration.device = s_device;
    surfaceConfiguration.format = s_mainSurfacePreferredFormat;
    surfaceConfiguration.width = s_window->renderSurfaceWidth();
    surfaceConfiguration.height = s_window->renderSurfaceHeight();
    s_surface.Configure(&surfaceConfiguration);

    wgpu::TextureDescriptor depthTextureDescriptor = {};
    depthTextureDescriptor.size = {static_cast<uint32_t>(s_window->renderSurfaceWidth()), static_cast<uint32_t>(s_window->renderSurfaceHeight()), 1};
    depthTextureDescriptor.format = wgpu::TextureFormat::Depth24Plus;
    depthTextureDescriptor.sampleCount = multisampleCount;
    depthTextureDescriptor.usage = wgpu::TextureUsage::RenderAttachment;
    s_depthAttachment.view = s_device.CreateTexture(&depthTextureDescriptor).CreateView();

    if(multisampleCount != 1) {
        wgpu::TextureDescriptor multisampleTextureDescriptor = {};
        multisampleTextureDescriptor.size = {static_cast<uint32_t>(s_window->renderSurfaceWidth()), static_cast<uint32_t>(s_window->renderSurfaceHeight()), 1};
        multisampleTextureDescriptor.format = s_mainSurfacePreferredFormat;
        multisampleTextureDescriptor.sampleCount = multisampleCount;
        multisampleTextureDescriptor.usage = wgpu::TextureUsage::RenderAttachment;
        s_colorAttachment.view = s_device.CreateTexture(&multisampleTextureDescriptor).CreateView();
    }
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

    wgpu::SurfaceTexture currentSurfaceTexture;
    s_surface.GetCurrentTexture(&currentSurfaceTexture);

    if (multisampleCount == 1) {
        s_colorAttachment.view = currentSurfaceTexture.texture.CreateView();
    } else {
        s_colorAttachment.resolveTarget = currentSurfaceTexture.texture.CreateView();
    }

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

void WebGPURenderer::setUpCameraBuffer() {
    wgpu::BufferDescriptor bufferDescriptor = {};
    bufferDescriptor.size = 64 * 2;
    bufferDescriptor.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
    bufferDescriptor.mappedAtCreation = false;
    s_cameraDataBuffer = s_device.CreateBuffer(&bufferDescriptor);

    wgpu::BindGroupEntry bindGroupDescriptorEntry0 = {};
    bindGroupDescriptorEntry0.binding = 0;
    bindGroupDescriptorEntry0.buffer = GameEngine::WebGPURenderer::cameraDataBuffer();

    wgpu::BindGroupDescriptor bindGroupDescriptor = {};
    bindGroupDescriptor.layout = s_pbrRenderPipeline.GetBindGroupLayout(0);
    bindGroupDescriptor.entryCount = 1;
    bindGroupDescriptor.entries = &bindGroupDescriptorEntry0;

    s_cameraDataBindGroup = s_device.CreateBindGroup(&bindGroupDescriptor);
}

void WebGPURenderer::setUpPBRRenderPipeline() {
    auto device = GameEngine::WebGPURenderer::device();

    wgpu::RenderPipelineDescriptor pipelineDescriptor = {};

    pipelineDescriptor.layout = nullptr; // auto layout

    wgpu::ColorTargetState colorTargetState = {};
    colorTargetState.format = GameEngine::WebGPURenderer::mainSurfacePreferredFormat();

    int pbrRenderShaderHandle = AssetManager::loadShader("shaders/basic_color.wgsl");
    auto& shader = AssetManager::getShader(pbrRenderShaderHandle);

    wgpu::FragmentState fragment = {};
    fragment.module = shader.shaderModule();
    fragment.entryPoint = "frag";
    fragment.targetCount = 1;
    fragment.targets = &colorTargetState;

    wgpu::VertexAttribute positionBufferLayoutAttribute0 = {};
    positionBufferLayoutAttribute0.shaderLocation = 0;
    positionBufferLayoutAttribute0.offset = 0;
    positionBufferLayoutAttribute0.format = wgpu::VertexFormat::Float32x3;

    wgpu::VertexBufferLayout positionBufferLayout = {};
    positionBufferLayout.arrayStride = 3 * 4;
    positionBufferLayout.attributeCount = 1;
    positionBufferLayout.attributes = &positionBufferLayoutAttribute0;

    wgpu::VertexAttribute normalBufferLayoutAttribute0 = {};
    normalBufferLayoutAttribute0.shaderLocation = 1;
    normalBufferLayoutAttribute0.offset = 0;
    normalBufferLayoutAttribute0.format = wgpu::VertexFormat::Float32x3;

    wgpu::VertexBufferLayout normalBufferLayout = {};
    normalBufferLayout.arrayStride = 3 * 4;
    normalBufferLayout.attributeCount = 1;
    normalBufferLayout.attributes = &normalBufferLayoutAttribute0;

    wgpu::VertexAttribute uvBufferLayoutAttribute0 = {};
    uvBufferLayoutAttribute0.shaderLocation = 2;
    uvBufferLayoutAttribute0.offset = 0;
    uvBufferLayoutAttribute0.format = wgpu::VertexFormat::Float32x2;

    wgpu::VertexBufferLayout uvBufferLayout = {};
    uvBufferLayout.arrayStride = 2 * 4;
    uvBufferLayout.attributeCount = 1;
    uvBufferLayout.attributes = &uvBufferLayoutAttribute0;

    wgpu::VertexAttribute tangentBufferLayoutAttribute0 = {};
    tangentBufferLayoutAttribute0.shaderLocation = 3;
    tangentBufferLayoutAttribute0.offset = 0;
    tangentBufferLayoutAttribute0.format = wgpu::VertexFormat::Float32x4;

    wgpu::VertexBufferLayout tangentBufferLayout = {};
    tangentBufferLayout.arrayStride = 4 * 4;
    tangentBufferLayout.attributeCount = 1;
    tangentBufferLayout.attributes = &tangentBufferLayoutAttribute0;

    std::array<wgpu::VertexBufferLayout, 4> vertexBufferLayouts = {positionBufferLayout, normalBufferLayout, uvBufferLayout, tangentBufferLayout};

    wgpu::VertexState vertex = {};
    vertex.module = shader.shaderModule();
    vertex.entryPoint = "vert";
    vertex.bufferCount = vertexBufferLayouts.size();
    vertex.buffers = vertexBufferLayouts.data();

    pipelineDescriptor.vertex = vertex;
    pipelineDescriptor.fragment = &fragment;

    pipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
    pipelineDescriptor.primitive.cullMode = wgpu::CullMode::Back;

    pipelineDescriptor.multisample.count = multisampleCount;

    wgpu::DepthStencilState depthStencilState = {};
    depthStencilState.depthWriteEnabled = true;
    depthStencilState.depthCompare = wgpu::CompareFunction::Less;
    depthStencilState.format = wgpu::TextureFormat::Depth24Plus;

    pipelineDescriptor.depthStencil = &depthStencilState;

    s_pbrRenderPipeline = device.CreateRenderPipeline(&pipelineDescriptor);
}

void WebGPURenderer::updateCameraDataBuffer(const glm::mat4 &view, const glm::mat4 &projection) {
    uint8_t data[128];
    std::memcpy(data, glm::value_ptr(view), 64);
    std::memcpy(data + 64, glm::value_ptr(projection), 64);
    device().GetQueue().WriteBuffer(s_cameraDataBuffer, 0, data, 128);
}

void WebGPURenderer::renderMesh(Entity &entity, const PBRRendererComponent &renderer, const WebGPUPBRRendererDataComponent &rendererData) {
    uint8_t data[128];
    std::memcpy(data, glm::value_ptr(entity.globalModelMatrix()), 64);
    std::memcpy(data + 64, glm::value_ptr(renderer.color), 16);
    GameEngine::WebGPURenderer::device().GetQueue().WriteBuffer(rendererData.objectDataBuffer, 0, data, 64 + 16);

    auto& mesh = GameEngine::AssetManager::getMesh(renderer.meshHandle);

    auto renderPassEncoder = GameEngine::WebGPURenderer::renderPassEncoder();
    renderPassEncoder.SetPipeline(s_pbrRenderPipeline);

    renderPassEncoder.SetBindGroup(0, s_cameraDataBindGroup);
    renderPassEncoder.SetBindGroup(1, s_materialBindGroup);
    renderPassEncoder.SetBindGroup(2, rendererData.objectDataBindGroup);

    renderPassEncoder.SetVertexBuffer(0, mesh.positionBuffer());
    renderPassEncoder.SetVertexBuffer(1, mesh.normalBuffer());
    renderPassEncoder.SetVertexBuffer(2, mesh.uvBuffer());
    renderPassEncoder.SetVertexBuffer(3, mesh.tangentBuffer());

    renderPassEncoder.SetIndexBuffer(mesh.indexBuffer(), wgpu::IndexFormat::Uint32);

    renderPassEncoder.DrawIndexed(mesh.indexCount());
}

WebGPUPBRRendererDataComponent::WebGPUPBRRendererDataComponent() {
    auto &device = WebGPURenderer::device();

    wgpu::BufferDescriptor bufferDescriptor = {};
    bufferDescriptor.size = 64 + 16;
    bufferDescriptor.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
    objectDataBuffer = device.CreateBuffer(&bufferDescriptor);

    wgpu::BindGroupEntry bindGroupDescriptorEntry0 = {};
    bindGroupDescriptorEntry0.binding = 0;
    bindGroupDescriptorEntry0.buffer = objectDataBuffer;

    wgpu::BindGroupDescriptor bindGroupDescriptor = {};
    bindGroupDescriptor.layout = s_pbrRenderPipeline.GetBindGroupLayout(2);
    bindGroupDescriptor.entryCount = 1;
    bindGroupDescriptor.entries = &bindGroupDescriptorEntry0;

    objectDataBindGroup = device.CreateBindGroup(&bindGroupDescriptor);
}

}
