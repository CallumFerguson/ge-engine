#include "WebGPURenderer.hpp"

#include <vector>
#include <map>
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
#include "../../CubeMap.hpp"
#include "../../../Utility/TimingHelper.hpp"

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

static wgpu::RenderPassColorAttachment s_colorAttachment;
static wgpu::RenderPassDepthStencilAttachment s_depthAttachment;
static wgpu::RenderPassDescriptor s_renderPassDescriptor;

static wgpu::RenderPassEncoder s_renderPassEncoder;
static wgpu::CommandEncoder s_commandEncoder;

static wgpu::Buffer s_cameraDataBuffer;

struct MeshRenderInfo {
    Mesh &mesh;
    Material &material;
    const wgpu::BindGroup& objectDataBindGroup;
};

static std::vector<MeshRenderInfo> s_opaqueMeshesToRender;
static std::vector<MeshRenderInfo> s_transparentMeshesToRender;

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

    bool canTimestamp = std::find(features.begin(), features.end(), wgpu::FeatureName::TimestampQuery) != features.end();
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

    if(s_window) {
        finishInit();
    }

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

    setUpCameraBuffer();

    WebGPUShader::registerShaderCreatePipelineFunction(BASIC_COLOR_SHADER_UUID, createPBRRenderPipeline);
    WebGPUShader::registerShaderCreatePipelineFunction(PBR_SHADER_UUID, createPBRRenderPipeline);
    WebGPUShader::registerShaderCreatePipelineFunction(SKYBOX_SHADER_UUID, [](const wgpu::ShaderModule &shaderModule, bool depthWrite) {
        return WebGPURenderer::createBasicPipeline(shaderModule, true, depthWrite);
    });
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
            // when app exits, this triggers. TODO: when clicking "x" on window, clean up properly
//            std::cout << "instance dropped" << std::endl;
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

    // buffer

    wgpu::BufferDescriptor bufferDescriptor = {};
    bufferDescriptor.size = 208;
    bufferDescriptor.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
    bufferDescriptor.mappedAtCreation = false;
    s_cameraDataBuffer = s_device.CreateBuffer(&bufferDescriptor);

//    // layout

//    wgpu::BufferBindingLayout cameraDataBindGroupLayoutEntry0BufferBindingLayout = {};
//    cameraDataBindGroupLayoutEntry0BufferBindingLayout.type = wgpu::BufferBindingType::Uniform;
//
//    wgpu::BindGroupLayoutEntry cameraDataBindGroupLayoutEntry0 = {};
//    cameraDataBindGroupLayoutEntry0.binding = 0;
//    cameraDataBindGroupLayoutEntry0.visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
//    cameraDataBindGroupLayoutEntry0.buffer = cameraDataBindGroupLayoutEntry0BufferBindingLayout;
//
//    wgpu::BindGroupLayoutDescriptor cameraDataBindGroupLayoutDescriptor = {};
//    cameraDataBindGroupLayoutDescriptor.entryCount = 1;
//    cameraDataBindGroupLayoutDescriptor.entries = &cameraDataBindGroupLayoutEntry0;
//    s_cameraDataBindGroupLayout = s_device.CreateBindGroupLayout(&cameraDataBindGroupLayoutDescriptor);

//    // bind group
//    wgpu::BindGroupEntry bindGroupDescriptorEntry0 = {};
//    bindGroupDescriptorEntry0.binding = 0;
//    bindGroupDescriptorEntry0.buffer = GameEngine::WebGPURenderer::cameraDataBuffer();
//
//    wgpu::BindGroupDescriptor bindGroupDescriptor = {};
//    bindGroupDescriptor.layout = ;
//    bindGroupDescriptor.entryCount = 1;
//    bindGroupDescriptor.entries = &bindGroupDescriptorEntry0;
//
//    s_cameraDataBindGroup = s_device.CreateBindGroup(&bindGroupDescriptor);
}

wgpu::RenderPipeline WebGPURenderer::createPBRRenderPipeline(const wgpu::ShaderModule& shaderModule, bool depthWrite) {
    auto &device = GameEngine::WebGPURenderer::device();

    wgpu::RenderPipelineDescriptor pipelineDescriptor = {};

    pipelineDescriptor.layout = nullptr; // auto layout

    wgpu::BlendState blendState;
    blendState.color.operation = wgpu::BlendOperation::Add;
    blendState.color.srcFactor = wgpu::BlendFactor::SrcAlpha;
    blendState.color.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;

    blendState.alpha.operation = wgpu::BlendOperation::Add;
    blendState.alpha.srcFactor = wgpu::BlendFactor::SrcAlpha;
    blendState.alpha.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;

    wgpu::ColorTargetState colorTargetState = {};
    colorTargetState.format = GameEngine::WebGPURenderer::mainSurfacePreferredFormat();
    if(!depthWrite) {
        colorTargetState.blend = &blendState;
    }

    wgpu::FragmentState fragment = {};
    fragment.module = shaderModule;
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
    vertex.module = shaderModule;
    vertex.entryPoint = "vert";
    vertex.bufferCount = vertexBufferLayouts.size();
    vertex.buffers = vertexBufferLayouts.data();

    pipelineDescriptor.vertex = vertex;
    pipelineDescriptor.fragment = &fragment;

    pipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
    pipelineDescriptor.primitive.cullMode = wgpu::CullMode::Back;

    pipelineDescriptor.multisample.count = multisampleCount;

    wgpu::DepthStencilState depthStencilState = {};
    depthStencilState.depthWriteEnabled = depthWrite;
    depthStencilState.depthCompare = wgpu::CompareFunction::Less;
    depthStencilState.format = wgpu::TextureFormat::Depth24Plus;

    pipelineDescriptor.depthStencil = &depthStencilState;

    return device.CreateRenderPipeline(&pipelineDescriptor);
}

void WebGPURenderer::updateCameraDataBuffer(Entity &entity, TransformComponent &transform, CameraComponent &camera) {
    auto view = CameraComponent::modelToView(entity.globalModelMatrix(true));

    auto projection = camera.projection();

    auto cameraPosition = glm::vec3(entity.globalModelMatrix(true)[3]);

    auto viewAtOrigin = view;
    viewAtOrigin[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    auto viewDirectionProjectionInverse = glm::inverse(projection * viewAtOrigin);

    uint8_t data[208];
    std::memcpy(data, glm::value_ptr(view), 64);
    std::memcpy(data + 64, glm::value_ptr(projection), 64);
    std::memcpy(data + 128, glm::value_ptr(cameraPosition), 12);
    std::memcpy(data + 140, &camera.exposure, 4);
    std::memcpy(data + 144, glm::value_ptr(viewDirectionProjectionInverse), 64);
    device().GetQueue().WriteBuffer(s_cameraDataBuffer, 0, data, 208);
}

void WebGPURenderer::submitMeshToRenderer(Entity &entity, const PBRRendererComponent &renderer, const WebGPUPBRRendererDataComponent &rendererData) {
    glm::mat4 normalMatrix = glm::transpose(glm::inverse(entity.globalModelMatrix()));

    std::array<uint8_t, 64 + 64 + 16> data;
    std::memcpy(data.data(), glm::value_ptr(entity.globalModelMatrix()), 64);
    std::memcpy(data.data() + 64, glm::value_ptr(normalMatrix), 64);
    std::memcpy(data.data() + 128, glm::value_ptr(renderer.color), 12);
    GameEngine::WebGPURenderer::device().GetQueue().WriteBuffer(rendererData.objectDataBuffer, 0, data.data(), data.size());

    auto& mesh = GameEngine::AssetManager::getAsset<Mesh>(renderer.meshHandle);
    auto& material = GameEngine::AssetManager::getAsset<Material>(renderer.materialHandle);

    switch (material.renderQueue) {
        case RenderQueue::Opaque:
            s_opaqueMeshesToRender.push_back({mesh, material, rendererData.objectDataBindGroup});
            break;
        case RenderQueue::Transparent:
            s_transparentMeshesToRender.push_back({mesh, material, rendererData.objectDataBindGroup});
            break;
        default:
            std::cout << "material materialBindGroup unknown render queue: " << static_cast<uint8_t>(material.renderQueue) << std::endl;
            break;
    }
}

void renderMesh(MeshRenderInfo &meshRenderInfo) {
    auto& mesh = meshRenderInfo.mesh;
    auto& material= meshRenderInfo.material;
    auto& objectDataBindGroup = meshRenderInfo.objectDataBindGroup;

    auto renderPassEncoder = GameEngine::WebGPURenderer::renderPassEncoder();
    renderPassEncoder.SetPipeline(material.renderPipeline());

    renderPassEncoder.SetBindGroup(0, material.cameraBindGroup());
    renderPassEncoder.SetBindGroup(1, material.materialBindGroup());
    renderPassEncoder.SetBindGroup(2, objectDataBindGroup);

    renderPassEncoder.SetVertexBuffer(0, mesh.positionBuffer());
    renderPassEncoder.SetVertexBuffer(1, mesh.normalBuffer());
    renderPassEncoder.SetVertexBuffer(2, mesh.uvBuffer());
    renderPassEncoder.SetVertexBuffer(3, mesh.tangentBuffer());

    renderPassEncoder.SetIndexBuffer(mesh.indexBuffer(), wgpu::IndexFormat::Uint32);

    renderPassEncoder.DrawIndexed(mesh.indexCount());
}

void WebGPURenderer::renderOpaqueMeshes() {
    for(auto& meshRenderInfo : s_opaqueMeshesToRender) {
        renderMesh(meshRenderInfo);
    }
    s_opaqueMeshesToRender.clear();
}

void WebGPURenderer::renderTransparentMeshes() {
    for(auto& meshRenderInfo : s_transparentMeshesToRender) {
        renderMesh(meshRenderInfo);
    }
    s_transparentMeshesToRender.clear();
}

wgpu::Sampler &WebGPURenderer::basicSampler() {
    static wgpu::Sampler s_sampler;
    if (!s_sampler) {
        wgpu::SamplerDescriptor samplerDescriptor;
        samplerDescriptor.magFilter = wgpu::FilterMode::Linear;
        samplerDescriptor.minFilter = wgpu::FilterMode::Linear;
        samplerDescriptor.mipmapFilter = wgpu::MipmapFilterMode::Linear;
        s_sampler = s_device.CreateSampler(&samplerDescriptor);
    }
    return s_sampler;
}

WebGPUPBRRendererDataComponent::WebGPUPBRRendererDataComponent(int materialHandle) {
    if(materialHandle == -1) {
        std::cout << "WebGPUPBRRendererDataComponent::WebGPUPBRRendererDataComponent materialHandle was -1" << std::endl;
        return;
    }

    auto& material = AssetManager::getAsset<Material>(materialHandle);
    auto& shader = AssetManager::getAsset<WebGPUShader>(material.shaderHandle);

    auto &device = WebGPURenderer::device();

    wgpu::BufferDescriptor bufferDescriptor = {};
    bufferDescriptor.size = 64 + 64 + 16;
    bufferDescriptor.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
    objectDataBuffer = device.CreateBuffer(&bufferDescriptor);

    {
        wgpu::BindGroupEntry bindGroupDescriptorEntry0 = {};
        bindGroupDescriptorEntry0.binding = 0;
        bindGroupDescriptorEntry0.buffer = objectDataBuffer;

        wgpu::BindGroupDescriptor bindGroupDescriptor = {};
        switch (material.renderQueue) {
            case RenderQueue::Opaque:
                bindGroupDescriptor.layout = shader.renderPipeline(true).GetBindGroupLayout(2);
                break;
            case RenderQueue::Transparent:
                bindGroupDescriptor.layout = shader.renderPipeline(false).GetBindGroupLayout(2);
                break;
            default:
                std::cout << "WebGPUPBRRendererDataComponent unknown render queue: " << static_cast<uint8_t>(material.renderQueue) << std::endl;
                bindGroupDescriptor.layout = shader.renderPipeline(true).GetBindGroupLayout(2);
                break;
        }
        bindGroupDescriptor.entryCount = 1;
        bindGroupDescriptor.entries = &bindGroupDescriptorEntry0;

        objectDataBindGroup = device.CreateBindGroup(&bindGroupDescriptor);
    }
}

void WebGPURenderer::renderSkybox(const Skybox &skybox) {
    auto& device = s_device;

    auto& cubeMap = AssetManager::getAsset<CubeMap>(skybox.cubeMapHandle);

    int shaderHandle = AssetManager::getOrLoadAssetFromUUID<WebGPUShader>(SKYBOX_SHADER_UUID);
    auto &shader = AssetManager::getAsset<WebGPUShader>(shaderHandle);

    s_renderPassEncoder.SetPipeline(shader.renderPipeline(true));
    s_renderPassEncoder.SetBindGroup(0, skybox.bindGroup());
    s_renderPassEncoder.Draw(3);
}

wgpu::RenderPipeline WebGPURenderer::createBasicPipeline(const wgpu::ShaderModule &shaderModule, bool renderToScreen, bool depthWrite, wgpu::TextureFormat textureFormat) {
    wgpu::RenderPipelineDescriptor pipelineDescriptor = {};

    pipelineDescriptor.layout = nullptr; // auto layout

    wgpu::ColorTargetState colorTargetState = {};
    if(renderToScreen) {
        colorTargetState.format = GameEngine::WebGPURenderer::mainSurfacePreferredFormat();
    } else {
        colorTargetState.format = textureFormat;
        if(textureFormat == wgpu::TextureFormat::Undefined) {
            std::cout << "createBasicPipeline renderToScreen was false, but textureFormat was Undefined" << std::endl;
        }
    }

    wgpu::FragmentState fragment = {};
    fragment.module = shaderModule;
    fragment.entryPoint = "frag";
    fragment.targetCount = 1;
    fragment.targets = &colorTargetState;

    wgpu::VertexState vertex = {};
    vertex.module = shaderModule;
    vertex.entryPoint = "vert";
    vertex.bufferCount = 0;

    pipelineDescriptor.vertex = vertex;
    pipelineDescriptor.fragment = &fragment;

    pipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
    pipelineDescriptor.primitive.cullMode = wgpu::CullMode::Back;

    if(renderToScreen) {
        pipelineDescriptor.multisample.count = multisampleCount;

        wgpu::DepthStencilState depthStencilState = {};
        depthStencilState.depthWriteEnabled = depthWrite;
        depthStencilState.depthCompare = wgpu::CompareFunction::LessEqual;
        depthStencilState.format = wgpu::TextureFormat::Depth24Plus;

        pipelineDescriptor.depthStencil = &depthStencilState;
    }

    return s_device.CreateRenderPipeline(&pipelineDescriptor);
}

}
