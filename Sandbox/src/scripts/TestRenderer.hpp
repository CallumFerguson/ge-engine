#pragma once

#include <memory>
#include <webgpu/webgpu_cpp.h>
#include "GameEngine.hpp"

struct TestRenderer : public GameEngine::ScriptableEntity {
public:
    explicit TestRenderer(std::shared_ptr<GameEngine::WebGPUShader> shader);

    void onStart();

    void onUpdate();

    void onImGui();

    void onMainRenderPass();

private:
    std::shared_ptr<GameEngine::WebGPUShader> m_shader;

    size_t m_numIndices = 0;

    wgpu::Buffer m_positionBuffer;
    wgpu::Buffer m_indexBuffer;
    wgpu::Buffer m_uniformBuffer;

    float m_uniformBufferData[4] = {0.25, 0, 0, 1};

    wgpu::RenderPipeline m_pipeline;

    wgpu::BindGroup m_bindGroup0;

    void randomizeColor();
};
