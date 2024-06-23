#include "TestRenderer.hpp"

#include <sstream>
#include <utility>
#include <imgui.h>
#include <imgui_memory_editor.h>
#include <glm/gtc/type_ptr.hpp>

TestRenderer::TestRenderer(int shaderHandle, int meshHandle): m_shaderHandle(shaderHandle), m_meshHandle(meshHandle) {}

void TestRenderer::onStart() {
    auto &device = GameEngine::WebGPURenderer::device();

    wgpu::RenderPipelineDescriptor pipelineDescriptor = {};

    pipelineDescriptor.layout = nullptr; // auto layout

    wgpu::ColorTargetState colorTargetState = {};
    colorTargetState.format = GameEngine::WebGPURenderer::mainSurfacePreferredFormat();

    auto& shader = GameEngine::AssetManager::getAsset<GameEngine::WebGPUShader>(m_shaderHandle);

    wgpu::FragmentState fragment = {};
    fragment.module = shader.shaderModule();
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
    vertex.module = shader.shaderModule();
    vertex.entryPoint = "vert";
    vertex.bufferCount = 1;
    vertex.buffers = &positionBufferLayout;

    pipelineDescriptor.vertex = vertex;
    pipelineDescriptor.fragment = &fragment;

    pipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
    pipelineDescriptor.primitive.cullMode = wgpu::CullMode::Back;

    pipelineDescriptor.multisample.count = 1;

    m_pipeline = device.CreateRenderPipeline(&pipelineDescriptor);

    // camera
    {
        wgpu::BindGroupEntry bindGroupDescriptorEntry0 = {};
        bindGroupDescriptorEntry0.binding = 0;
        bindGroupDescriptorEntry0.buffer = GameEngine::WebGPURenderer::cameraDataBuffer();

        wgpu::BindGroupDescriptor bindGroupDescriptor = {};
        bindGroupDescriptor.layout = m_pipeline.GetBindGroupLayout(0);
        bindGroupDescriptor.entryCount = 1;
        bindGroupDescriptor.entries = &bindGroupDescriptorEntry0;

        m_cameraDataBindGroup = device.CreateBindGroup(&bindGroupDescriptor);
    }

    // object data
    {
        wgpu::BufferDescriptor bufferDescriptor = {};
        bufferDescriptor.size = 64 + 16;
        bufferDescriptor.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
        m_objectDataBuffer = device.CreateBuffer(&bufferDescriptor);

        wgpu::BindGroupEntry bindGroupDescriptorEntry0 = {};
        bindGroupDescriptorEntry0.binding = 0;
        bindGroupDescriptorEntry0.buffer = m_objectDataBuffer;

        wgpu::BindGroupDescriptor bindGroupDescriptor = {};
        bindGroupDescriptor.layout = m_pipeline.GetBindGroupLayout(1);
        bindGroupDescriptor.entryCount = 1;
        bindGroupDescriptor.entries = &bindGroupDescriptorEntry0;

        m_objectDataBindGroup = device.CreateBindGroup(&bindGroupDescriptor);
    }
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

void TestRenderer::onMainRenderPass() {
    uint8_t data[128];
    std::memcpy(data, glm::value_ptr(getEntity().globalModelMatrix()), 64);
    std::memcpy(data + 64, glm::value_ptr(m_color), 16);
    GameEngine::WebGPURenderer::device().GetQueue().WriteBuffer(m_objectDataBuffer, 0, data, 64 + 16);

    auto& mesh = GameEngine::AssetManager::getAsset<GameEngine::Mesh>(m_meshHandle);

    auto renderPassEncoder = GameEngine::WebGPURenderer::renderPassEncoder();
    renderPassEncoder.SetPipeline(m_pipeline);
    renderPassEncoder.SetBindGroup(0, m_cameraDataBindGroup);
    renderPassEncoder.SetBindGroup(1, m_objectDataBindGroup);
    renderPassEncoder.SetVertexBuffer(0, mesh.positionBuffer());
    renderPassEncoder.SetIndexBuffer(mesh.indexBuffer(), wgpu::IndexFormat::Uint32);
    renderPassEncoder.DrawIndexed(mesh.indexCount());
}

void TestRenderer::randomizeColor() {
    m_color[0] = GameEngine::Random::value();
    m_color[1] = GameEngine::Random::value();
    m_color[2] = GameEngine::Random::value();
}

void TestRenderer::onImGuiInspector() {
    ImGui::Text("Mesh handle: %d", m_meshHandle);
    if (ImGui::TreeNode("color")) {
        auto colorPtr = glm::value_ptr(m_color);
        ImGui::ColorPicker3("##color picker", colorPtr);
        ImGui::TreePop();
    }
}

nlohmann::json TestRenderer::toJSON() {
    nlohmann::json result;
    result["color"] = {m_color[0], m_color[1], m_color[2], m_color[3]};
    std::cout << "TODO: save shader/mesh assets with guid. maybe mesh also has a toJSON" << std::endl;
    return result;
}

void TestRenderer::initFromJSON(const nlohmann::json &scriptJSON) {
    auto color = scriptJSON["color"].get<std::vector<float>>();
    for(int i = 0; i < 4; i++) {
        m_color[i] = color[i];
    }

    std::cout << "TODO: load shared shader/mesh assets" << std::endl;

//    m_shader = std::make_shared<GameEngine::WebGPUShader>("shaders/unlit_color.wgsl");
//    m_meshHandle = std::make_shared<GameEngine::Mesh>("assets/FlightHelmetPackaged/FlightHelmet.asset");
}
