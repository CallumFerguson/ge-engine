#include "TestRenderer.hpp"

#include <sstream>
#include <utility>
#include <imgui.h>
#include <imgui_memory_editor.h>
#include <glm/gtc/type_ptr.hpp>

TestRenderer::TestRenderer(std::shared_ptr<GameEngine::WebGPUShader> shader, std::shared_ptr<GameEngine::Mesh> mesh): m_shader(std::move(shader)), m_mesh(std::move(mesh)) {}

void TestRenderer::onStart() {
    auto device = GameEngine::WebGPURenderer::device();

    wgpu::ColorTargetState colorTargetState = {};
    colorTargetState.format = GameEngine::WebGPURenderer::mainSurfacePreferredFormat();

    wgpu::BufferBindingLayout objectDataBindGroupLayoutEntry0BufferBindingLayout = {};
    objectDataBindGroupLayoutEntry0BufferBindingLayout.type = wgpu::BufferBindingType::Uniform;

    wgpu::BindGroupLayoutEntry objectDataBindGroupLayoutEntry0 = {};
    objectDataBindGroupLayoutEntry0.binding = 0;
    objectDataBindGroupLayoutEntry0.visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
    objectDataBindGroupLayoutEntry0.buffer = objectDataBindGroupLayoutEntry0BufferBindingLayout;

    wgpu::BindGroupLayoutDescriptor objectDataBindGroupLayoutDescriptor = {};
    objectDataBindGroupLayoutDescriptor.entryCount = 1;
    objectDataBindGroupLayoutDescriptor.entries = &objectDataBindGroupLayoutEntry0;
    auto objectDataBindGroupLayout = device.CreateBindGroupLayout(&objectDataBindGroupLayoutDescriptor);

    wgpu::BindGroupLayout bindGroupLayouts[2] = {GameEngine::WebGPURenderer::cameraDataBindGroupLayout(), objectDataBindGroupLayout};

    wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor = {};
    pipelineLayoutDescriptor.bindGroupLayoutCount = 2;
    pipelineLayoutDescriptor.bindGroupLayouts = bindGroupLayouts;
    auto pipelineLayout = device.CreatePipelineLayout(&pipelineLayoutDescriptor);

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
    pipelineDescriptor.primitive.cullMode = wgpu::CullMode::Back;

    pipelineDescriptor.multisample.count = 1;

    m_pipeline = device.CreateRenderPipeline(&pipelineDescriptor);

    wgpu::BufferDescriptor uniformBufferDescriptor = {};
    uniformBufferDescriptor.size = 64 + 16;
    uniformBufferDescriptor.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
    m_uniformBuffer = device.CreateBuffer(&uniformBufferDescriptor);

    wgpu::BindGroupEntry objectDataBindGroupDescriptorEntry0 = {};
    objectDataBindGroupDescriptorEntry0.binding = 0;
    objectDataBindGroupDescriptorEntry0.buffer = m_uniformBuffer;

    wgpu::BindGroupDescriptor objectDataBindGroupDescriptor = {};
    objectDataBindGroupDescriptor.layout = objectDataBindGroupLayout;
    objectDataBindGroupDescriptor.entryCount = 1;
    objectDataBindGroupDescriptor.entries = &objectDataBindGroupDescriptorEntry0;

    m_objectDataBindGroup = device.CreateBindGroup(&objectDataBindGroupDescriptor);
}

void TestRenderer::onUpdate() {
    if(GameEngine::Input::getKeyDown(GameEngine::KeyCode::Space)) {
        randomizeColor();
    }
    if(GameEngine::Input::getKey(GameEngine::KeyCode::R)) {
        randomizeColor();
    }
    const float speed = 10;
    if(GameEngine::Input::getKey(GameEngine::KeyCode::Up)) {
        getComponent<GameEngine::TransformComponent>().localPosition[1] += GameEngine::Time::deltaTime() * speed;
    }
    if(GameEngine::Input::getKey(GameEngine::KeyCode::Down)) {
        getComponent<GameEngine::TransformComponent>().localPosition[1] -= GameEngine::Time::deltaTime() * speed;
    }
    if(GameEngine::Input::getKey(GameEngine::KeyCode::Left)) {
        getComponent<GameEngine::TransformComponent>().localPosition[0] -= GameEngine::Time::deltaTime() * speed;
    }
    if(GameEngine::Input::getKey(GameEngine::KeyCode::Right)) {
        getComponent<GameEngine::TransformComponent>().localPosition[0] += GameEngine::Time::deltaTime() * speed;
    }
}

void TestRenderer::onImGui() {
    auto &name = getComponent<GameEngine::NameComponent>().name;

    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 10.0f, 10.0f), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    ImGui::Begin("Controls", nullptr,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    std::string buttonLabel = "Randomize " + name + " color";
    if (ImGui::Button(buttonLabel.c_str())) {
        randomizeColor();
    }
    ImGui::End();

//    static MemoryEditor mem_edit;
//    mem_edit.DrawWindow("Memory Editor", m_uniformBufferData, 16);
}

void TestRenderer::onMainRenderPass() {
    uint8_t data[128];
    std::memcpy(data, glm::value_ptr(getEntity().globalModelMatrix()), 64);
    std::memcpy(data + 64, glm::value_ptr(m_color), 16);
    GameEngine::WebGPURenderer::device().GetQueue().WriteBuffer(m_uniformBuffer, 0, data, 64 + 16);

    auto renderPassEncoder = GameEngine::WebGPURenderer::renderPassEncoder();
    renderPassEncoder.SetPipeline(m_pipeline);
    renderPassEncoder.SetBindGroup(0, GameEngine::WebGPURenderer::cameraDataBindGroup());
    renderPassEncoder.SetBindGroup(1, m_objectDataBindGroup);
    renderPassEncoder.SetVertexBuffer(0, m_mesh->positionBuffer());
    renderPassEncoder.SetIndexBuffer(m_mesh->indexBuffer(), wgpu::IndexFormat::Uint32);
    renderPassEncoder.DrawIndexed(m_mesh->indexCount());
}

void TestRenderer::randomizeColor() {
    m_color[0] = GameEngine::Random::value();
    m_color[1] = GameEngine::Random::value();
    m_color[2] = GameEngine::Random::value();
}

std::string &TestRenderer::imGuiName() {
    static std::string s_name = "TestRenderer";
    return s_name;
}

void TestRenderer::onImGuiInspector() {
    auto colorPtr = glm::value_ptr(m_color);
    if(ImGui::ColorPicker3("color", colorPtr)) {
        GameEngine::WebGPURenderer::device().GetQueue().WriteBuffer(m_uniformBuffer, 64, colorPtr, 16);
    }
}
