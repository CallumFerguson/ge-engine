#pragma once

#include <memory>
#include <webgpu/webgpu_cpp.h>
#include <glm/glm.hpp>
#include "GameEngine.hpp"

struct TestRenderer : public GameEngine::ScriptableEntity {
public:
    explicit TestRenderer(std::shared_ptr<GameEngine::WebGPUShader> shader, std::shared_ptr<GameEngine::Mesh> mesh);

    void onStart();

    void onUpdate();

    void onImGui();

    void onMainRenderPass();

    void onImGuiInspector() override;

    [[nodiscard]] const char *objectName() const override {
        return "TestRenderer";
    }

private:
    std::shared_ptr<GameEngine::WebGPUShader> m_shader;
    std::shared_ptr<GameEngine::Mesh> m_mesh;

    wgpu::Buffer m_uniformBuffer;

    glm::vec4 m_color{0.25f, 0.0f, 0.0f, 1.0f};

    wgpu::RenderPipeline m_pipeline;

    wgpu::BindGroup m_objectDataBindGroup;

    void randomizeColor();
};
