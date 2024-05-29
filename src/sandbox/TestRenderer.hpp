#pragma once

#include <memory>
#include <webgpu/webgpu_cpp.h>
#include "../engine/ScriptableEntity.hpp"
#include "../rendering/backends/webgpu/WebGPUShader.hpp"

struct TestRenderer : public ScriptableEntity {
public:
    explicit TestRenderer(std::shared_ptr<WebGPUShader> shader);

    void onStart();

    void onUpdate();

    void onImGui();

    void onRender();

private:
    std::shared_ptr<WebGPUShader> m_shader;

    size_t m_numIndices = 0;

    wgpu::Buffer m_positionBuffer;
    wgpu::Buffer m_indexBuffer;
    wgpu::Buffer m_uniformBuffer;

    float m_uniformBufferData[4] = {0.25, 0, 0, 1};

    wgpu::RenderPipeline m_pipeline;

    wgpu::BindGroup m_bindGroup0;

    void randomizeColor();
};
