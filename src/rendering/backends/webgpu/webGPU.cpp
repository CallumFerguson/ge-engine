#include "webGPU.hpp"

#include <iostream>
#include <fstream>
#include <random>
#include <optional>
#include <cmath>
#include <chrono>
#include <stdexcept>
#include <webgpu/webgpu_cpp.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_wgpu.h"
#include <GLFW/glfw3.h>
#include "entt/entt.hpp"
#include "imgui_memory_editor.h"

#include "../../../utility/RollingAverage.hpp"
#include "../../../assets/gltfloader.hpp"
#include "../../../utility/utility.hpp"

#ifdef __EMSCRIPTEN__

#include <emscripten.h>
#include <emscripten/html5_webgpu.h>

#include "../../../utility/emscriptenUtility.hpp"

#else

#include <webgpu/webgpu_glfw.h>

#endif

wgpu::Instance instance;
wgpu::Adapter adapter;
wgpu::Device device;
wgpu::Surface surface;
wgpu::RenderPipeline pipeline;
wgpu::BindGroup bindGroup0;
wgpu::Buffer uniformBuffer;
float uniformBufferData[4] = { 0.25, 0, 0, 1};
wgpu::Buffer positionBuffer;
wgpu::Buffer indexBuffer;

wgpu::RenderPassColorAttachment colorAttachment;
wgpu::RenderPassDescriptor renderPassDescriptor;

wgpu::TextureFormat presentationFormat;

size_t numIndices;

GLFWwindow* window;

#ifdef __EMSCRIPTEN__
wgpu::SwapChain swapChain;
#endif

std::optional<std::chrono::time_point<std::chrono::high_resolution_clock>> startTime = std::nullopt;

bool once = true;

double deltaTime;
RollingAverage fpsRollingAverage;

int renderSurfaceWidth = 512;
int renderSurfaceHeight = 512;

std::random_device rd;
std::mt19937 mt(rd());
std::uniform_real_distribution<float> dist(0.0f, 1.0f);

void randomizeColor(float color[4]) {
    color[0] = dist(mt);
    color[1] = dist(mt);
    color[2] = dist(mt);
}

void configureSurface() {
    // TODO: use configure api for both https://github.com/emscripten-core/emscripten/pull/21939
#ifdef __EMSCRIPTEN__
    wgpu::SwapChainDescriptor swapChainDescriptor = {};
    swapChainDescriptor.usage = wgpu::TextureUsage::RenderAttachment;
    swapChainDescriptor.format = presentationFormat;
    swapChainDescriptor.width = renderSurfaceWidth;
    swapChainDescriptor.height = renderSurfaceHeight;
    swapChainDescriptor.presentMode = wgpu::PresentMode::Fifo;
    swapChain = device.CreateSwapChain(surface, &swapChainDescriptor);
#else
    wgpu::SurfaceConfiguration surfaceConfiguration = {};
    surfaceConfiguration.device = device;
    surfaceConfiguration.format = presentationFormat;
    surfaceConfiguration.width = renderSurfaceWidth;
    surfaceConfiguration.height = renderSurfaceHeight;
    surface.Configure(&surfaceConfiguration);
#endif
}

void drawImGui() {
    ImGui_ImplWGPU_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

//    static bool showDemoWindow = true;
//    if (showDemoWindow) {
//        ImGui::ShowDemoWindow(&showDemoWindow);
//    }

    ImGui::SetNextWindowSize(ImVec2(0, 0));
    ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_Always, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Info", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
    ImGui::Text("fps: %d", static_cast<int>(std::round(fpsRollingAverage.average())));
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 10.0f, 10.0f), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    ImGui::Begin("Controls", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    if (ImGui::Button("Randomize color")) {
        randomizeColor(uniformBufferData);
        device.GetQueue().WriteBuffer(uniformBuffer, 0, uniformBufferData, 16);
    }
    ImGui::End();

//    static MemoryEditor mem_edit;
//    mem_edit.DrawWindow("Memory Editor", testData, 100);

    ImGui::Render();
}

void draw() {
#ifndef __EMSCRIPTEN__
    // Tick needs to be called in Dawn to display validation errors
    // https://github.com/ocornut/imgui/blob/master/examples/example_glfw_wgpu/main.cpp
    device.Tick();
#endif

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
    renderPassEncoder.SetBindGroup(0, bindGroup0);
    renderPassEncoder.SetVertexBuffer(0, positionBuffer);
    renderPassEncoder.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint16);
    renderPassEncoder.DrawIndexed(numIndices);

    ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), renderPassEncoder.Get());

    renderPassEncoder.End();

    auto commandBuffer = commandEncoder.Finish();
    device.GetQueue().Submit(1, &commandBuffer);
}

void mainLoop() {
//    glfwPollEvents();
//
//    // React to changes in screen size
//    int currentRenderSurfaceWidth, currentRenderSurfaceHeight;
//    glfwGetFramebufferSize(window, &currentRenderSurfaceWidth, &currentRenderSurfaceHeight);
//    if (currentRenderSurfaceWidth == 0 || currentRenderSurfaceHeight == 0) {
//        return;
//    }
//    if (currentRenderSurfaceWidth != renderSurfaceWidth || currentRenderSurfaceHeight != renderSurfaceHeight) {
//        ImGui_ImplWGPU_InvalidateDeviceObjects();
//        renderSurfaceWidth = currentRenderSurfaceWidth;
//        renderSurfaceHeight = currentRenderSurfaceHeight;
//        configureSurface();
//        ImGui_ImplWGPU_CreateDeviceObjects();
//    }

    drawImGui();
    draw();

#ifdef __EMSCRIPTEN__
    updateCursor();
#endif

#ifndef __EMSCRIPTEN__
    surface.Present();
    instance.ProcessEvents();
#endif
}

//#ifndef __EMSCRIPTEN__
//void windowPosCallback(GLFWwindow*, int xpos, int ypos) {
//    mainLoop();
//}
//
//void framebufferSizeCallback(GLFWwindow*, int width, int height) {
//    mainLoop();
//}
//#endif

void mainWebGPU() {
//    if (!glfwInit()) {
//        std::cout << "could not glfwInit" << std::endl;
//        return;
//    }
//
//    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
//    window = glfwCreateWindow(renderSurfaceWidth, renderSurfaceHeight, "WebGPU window", nullptr, nullptr);
//    if (!window) {
//        std::cout << "failed to create window" << std::endl;
//        return;
//    }
//
//    // in a browser, glfwCreateWindow will ignore the passed width and height,
//    // so get the actual size which is based on the canvas size
//#ifdef __EMSCRIPTEN__
//    glfwGetWindowSize(window, &renderSurfaceWidth, &renderSurfaceHeight);
//#endif

#ifdef __EMSCRIPTEN__
    wgpu::SurfaceDescriptorFromCanvasHTMLSelector canvasDescriptor = {};
    canvasDescriptor.selector = "#canvas";

    wgpu::SurfaceDescriptor surfaceDescriptor = {};
    surfaceDescriptor.nextInChain = &canvasDescriptor;

    surface = instance.CreateSurface(&surfaceDescriptor);
#else
    surface = wgpu::glfw::CreateSurfaceForWindow(instance, window);
#endif

//    IMGUI_CHECKVERSION();
//    ImGui::CreateContext();
//    ImGuiIO& io = ImGui::GetIO();
//    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
//    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
//
//    // this is only needed for emscripten, but to keep things consistent, just do it for all platforms.
//    // if init file is needed later, it can be added to emscripten by preloading/file packing it
//    io.IniFilename = nullptr;
//
//    ImGui::StyleColorsDark();
//
//    ImGui_ImplGlfw_InitForOther(window, true);
//
//#ifdef __EMSCRIPTEN__
//    ImGui_ImplGlfw_InstallEmscriptenCanvasResizeCallback("#canvas");
//#else
//    glfwSetWindowPosCallback(window, windowPosCallback);
//    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
//#endif

    presentationFormat = surface.GetPreferredFormat(adapter);

    configureSurface();

    ImGui_ImplWGPU_InitInfo init_info;
    init_info.Device = device.Get();
    init_info.NumFramesInFlight = 3;
    init_info.RenderTargetFormat = (WGPUTextureFormat)presentationFormat;
    init_info.DepthStencilFormat = WGPUTextureFormat_Undefined;
    ImGui_ImplWGPU_Init(&init_info);

    std::ifstream shaderFile("shaders/unlit_color.wgsl", std::ios::binary);
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

    wgpu::BufferBindingLayout bindGroupLayoutGroup0Entry0BufferBindingLayout = {};
    bindGroupLayoutGroup0Entry0BufferBindingLayout.type = wgpu::BufferBindingType::Uniform;

    wgpu::BindGroupLayoutEntry bindGroupLayoutGroup0Entry0 = {};
    bindGroupLayoutGroup0Entry0.binding = 0;
    bindGroupLayoutGroup0Entry0.visibility = wgpu::ShaderStage::Fragment;
    bindGroupLayoutGroup0Entry0.buffer = bindGroupLayoutGroup0Entry0BufferBindingLayout;

    wgpu::BindGroupLayoutDescriptor bindGroupLayoutGroup0Descriptor = {};
    bindGroupLayoutGroup0Descriptor.entryCount = 1;
    bindGroupLayoutGroup0Descriptor.entries = &bindGroupLayoutGroup0Entry0;
    auto bindGroupLayoutGroup0 = device.CreateBindGroupLayout(&bindGroupLayoutGroup0Descriptor);

    wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor = {};
    pipelineLayoutDescriptor.bindGroupLayoutCount = 1;
    pipelineLayoutDescriptor.bindGroupLayouts = &bindGroupLayoutGroup0;
    auto pipelineLayout = device.CreatePipelineLayout(&pipelineLayoutDescriptor);

    auto model = loadModel("assets/sphere.glb");
    if(!model.has_value()) {
        throw std::runtime_error("no model");
    }

    numIndices = model->numIndices;

    positionBuffer = createBuffer(device, model->positions, model->numPositions * 4 * 3, wgpu::BufferUsage::Vertex);
    indexBuffer = createBuffer(device, model->indices, model->numIndices * 2, wgpu::BufferUsage::Index);

    wgpu::RenderPipelineDescriptor pipelineDescriptor = {};

    pipelineDescriptor.layout = pipelineLayout;

    wgpu::FragmentState fragment = {};
    fragment.module = shaderModule;
    fragment.entryPoint = "frag";
    fragment.targetCount = 1;
    fragment.targets = &colorTargetState;

    wgpu::VertexAttribute vertexBuffer0Attribute0 = {};
    vertexBuffer0Attribute0.shaderLocation = 0;
    vertexBuffer0Attribute0.offset = 0;
    vertexBuffer0Attribute0.format = wgpu::VertexFormat::Float32x3;

    wgpu::VertexBufferLayout positionBufferLayout = {};
    positionBufferLayout.arrayStride = 3 * 4;
    positionBufferLayout.attributeCount = 1;
    positionBufferLayout.attributes = &vertexBuffer0Attribute0;

    wgpu::VertexState vertex = {};
    vertex.module = shaderModule;
    vertex.entryPoint = "vert";
    vertex.bufferCount = 1;
    vertex.buffers = &positionBufferLayout;

    pipelineDescriptor.vertex = vertex;
    pipelineDescriptor.fragment = &fragment;

    pipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
    pipelineDescriptor.primitive.cullMode = wgpu::CullMode::None;

    pipelineDescriptor.multisample.count = 1;

    pipeline = device.CreateRenderPipeline(&pipelineDescriptor);

    wgpu::BufferDescriptor uniformBufferDescriptor = {};
    uniformBufferDescriptor.size = 16;
    uniformBufferDescriptor.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
    uniformBuffer = device.CreateBuffer(&uniformBufferDescriptor);
    device.GetQueue().WriteBuffer(uniformBuffer, 0, uniformBufferData, uniformBufferDescriptor.size);

    wgpu::BindGroupEntry bindGroupDescriptor0Entry0 = {};
    bindGroupDescriptor0Entry0.binding = 0;
    bindGroupDescriptor0Entry0.buffer = uniformBuffer;

    wgpu::BindGroupDescriptor bindGroupDescriptor0 = {};
    bindGroupDescriptor0.layout = bindGroupLayoutGroup0;
    bindGroupDescriptor0.entryCount = 1;
    bindGroupDescriptor0.entries = &bindGroupDescriptor0Entry0;

    bindGroup0 = device.CreateBindGroup(&bindGroupDescriptor0);

    colorAttachment = {};
    colorAttachment.loadOp = wgpu::LoadOp::Clear;
    colorAttachment.storeOp = wgpu::StoreOp::Store;
    colorAttachment.clearValue = wgpu::Color{0, 0, 0, 1};

    renderPassDescriptor = {};
    renderPassDescriptor.colorAttachmentCount = 1;
    renderPassDescriptor.colorAttachments = &colorAttachment;

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(mainLoop, 0, false); // TODO: try simulate loop
#else
    while (!glfwWindowShouldClose(window)) {
        mainLoop();
    }
#endif
}
