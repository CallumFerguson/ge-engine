#pragma once

#include <memory>
#include <webgpu/webgpu_cpp.h>
#include <glm/glm.hpp>
#include "GameEngine.hpp"

struct TestRenderer : public GameEngine::ScriptableEntity {
public:
    TestRenderer() = default;

    explicit TestRenderer(int shaderHandle, int meshHandle);

    void onStart();

    void onUpdate();

    void onMainRenderPass();

    void onImGuiInspector() override;

    nlohmann::json toJSON();

    void initFromJSON(const nlohmann::json &scriptJSON);

    [[nodiscard]] const char *objectName() const override {
        return "TestRenderer";
    }

private:
    int m_shaderHandle = -1;
    int m_meshHandle = -1;

    glm::vec4 m_color{0.25f, 0.0f, 0.0f, 1.0f};

    wgpu::RenderPipeline m_pipeline;

    wgpu::BindGroup m_cameraDataBindGroup;

    wgpu::Buffer m_objectDataBuffer;
    wgpu::BindGroup m_objectDataBindGroup;

    void randomizeColor();
};
