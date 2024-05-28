#pragma once

#include <webgpu/webgpu_cpp.h>
#include "../engine/ScriptableEntity.hpp"

struct TestRenderer : public ScriptableEntity {
public:
    void onStart();

    void onUpdate();

    void onImGui();

    void onRender();

private:
    size_t m_numIndices;

    wgpu::Buffer m_positionBuffer;
    wgpu::Buffer m_indexBuffer;
    wgpu::Buffer m_uniformBuffer;

    float m_uniformBufferData[4] = {0.25, 0, 0, 1};

    wgpu::RenderPipeline m_pipeline;

    wgpu::BindGroup m_bindGroup0;

    void randomizeColor();
};
