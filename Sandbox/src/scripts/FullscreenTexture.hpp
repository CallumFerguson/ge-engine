#pragma once

#include <webgpu/webgpu_cpp.h>
#include "GameEngine.hpp"

class FullscreenTexture : public GameEngine::ScriptableEntity {
public:
    void onStart();

    void onMainRenderPass();

    [[nodiscard]] const char *objectName() const override {
        return "FullscreenTexture";
    }

private:
    int m_shaderHandle;

    wgpu::BindGroup m_bindGroup;
};
