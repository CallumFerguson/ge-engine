#include "TestRenderer.hpp"

#include <sstream>
#include <utility>
#include <imgui.h>
#include <imgui_memory_editor.h>

TestRenderer::TestRenderer(std::shared_ptr<GameEngine::WebGPUShader> shader): m_shader(std::move(shader)) {}

void TestRenderer::onStart() {
    auto device = GameEngine::WebGPURenderer::device();

    wgpu::ColorTargetState colorTargetState = {};
    colorTargetState.format = GameEngine::WebGPURenderer::mainSurfacePreferredFormat();

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

    GameEngine::MeshAsset meshAsset("assets/sphere.glb.asset");
    if(meshAsset.indices.empty()) {
        GameEngine::exitApp("no model");
    }

    m_numIndices = meshAsset.indices.size();

    m_positionBuffer = GameEngine::createWebGPUBuffer(device, meshAsset.positions.data(), meshAsset.positions.size() * sizeof(float) * 3, wgpu::BufferUsage::Vertex);
    m_indexBuffer = GameEngine::createWebGPUBuffer(device, meshAsset.indices.data(), meshAsset.indices.size() * sizeof(uint32_t), wgpu::BufferUsage::Index);

    wgpu::RenderPipelineDescriptor pipelineDescriptor = {};

    pipelineDescriptor.layout = pipelineLayout;

    wgpu::FragmentState fragment = {};
    fragment.module = m_shader->shaderModule();
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
    vertex.module = m_shader->shaderModule();
    vertex.entryPoint = "vert";
    vertex.bufferCount = 1;
    vertex.buffers = &positionBufferLayout;

    pipelineDescriptor.vertex = vertex;
    pipelineDescriptor.fragment = &fragment;

    pipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
    pipelineDescriptor.primitive.cullMode = wgpu::CullMode::None;

    pipelineDescriptor.multisample.count = 1;

    m_pipeline = device.CreateRenderPipeline(&pipelineDescriptor);

    wgpu::BufferDescriptor uniformBufferDescriptor = {};
    uniformBufferDescriptor.size = 16;
    uniformBufferDescriptor.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
    m_uniformBuffer = device.CreateBuffer(&uniformBufferDescriptor);
    device.GetQueue().WriteBuffer(m_uniformBuffer, 0, m_uniformBufferData, uniformBufferDescriptor.size);

    wgpu::BindGroupEntry bindGroupDescriptor0Entry0 = {};
    bindGroupDescriptor0Entry0.binding = 0;
    bindGroupDescriptor0Entry0.buffer = m_uniformBuffer;

    wgpu::BindGroupDescriptor bindGroupDescriptor0 = {};
    bindGroupDescriptor0.layout = bindGroupLayoutGroup0;
    bindGroupDescriptor0.entryCount = 1;
    bindGroupDescriptor0.entries = &bindGroupDescriptor0Entry0;

    m_bindGroup0 = device.CreateBindGroup(&bindGroupDescriptor0);
}

void TestRenderer::onUpdate() {
    if(GameEngine::Input::getKeyDown(GameEngine::KeyCode::Space)) {
        randomizeColor();
    }
    if(GameEngine::Input::getKey(GameEngine::KeyCode::R)) {
        randomizeColor();
    }
}

void TestRenderer::onImGui() {
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 10.0f, 10.0f), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    ImGui::Begin("Controls", nullptr,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    if (ImGui::Button("Randomize color")) {
        randomizeColor();
    }
    ImGui::End();

//    static MemoryEditor mem_edit;
//    mem_edit.DrawWindow("Memory Editor", m_uniformBufferData, 16);
}

void TestRenderer::onMainRenderPass() {
    auto renderPassEncoder = GameEngine::WebGPURenderer::renderPassEncoder();
    renderPassEncoder.SetPipeline(m_pipeline);
    renderPassEncoder.SetBindGroup(0, m_bindGroup0);
    renderPassEncoder.SetVertexBuffer(0, m_positionBuffer);
    renderPassEncoder.SetIndexBuffer(m_indexBuffer, wgpu::IndexFormat::Uint32);
    renderPassEncoder.DrawIndexed(m_numIndices);
}

void TestRenderer::randomizeColor() {
    m_uniformBufferData[0] = GameEngine::Random::value();
    m_uniformBufferData[1] = GameEngine::Random::value();
    m_uniformBufferData[2] = GameEngine::Random::value();
    GameEngine::WebGPURenderer::device().GetQueue().WriteBuffer(m_uniformBuffer, 0, m_uniformBufferData, 16);
}
