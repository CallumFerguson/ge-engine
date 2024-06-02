#pragma once

#include <memory>
#include <webgpu/webgpu_cpp.h>
#include "GameEngine.hpp"

struct TestRenderer : public GameEngine::ScriptableEntity {
public:
    explicit TestRenderer(std::shared_ptr<GameEngine::WebGPUShader> shader, std::shared_ptr<GameEngine::Mesh> mesh);

    void onStart();

    void onUpdate();

    void onImGui();

    void onMainRenderPass();

private:
    std::shared_ptr<GameEngine::WebGPUShader> m_shader;
    std::shared_ptr<GameEngine::Mesh> m_mesh;

    wgpu::Buffer m_uniformBuffer;

    float m_uniformBufferData[4] = {0.25, 0, 0, 1};

    wgpu::RenderPipeline m_pipeline;

    wgpu::BindGroup m_bindGroup0;

    void randomizeColor();
};
