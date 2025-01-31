#pragma once

#include <vector>
#include <string>
#include <webgpu/webgpu_cpp.h>
#include "../Assets/Asset.hpp"

namespace GameEngine {

enum class RenderQueue : uint8_t {
    Opaque = 0,
    Transparent = 1,
};

class Material : public Asset {
public:
    int shaderHandle = -1;

    RenderQueue renderQueue = RenderQueue::Opaque;

    Material();

    explicit Material(const std::string &assetPath);

    void addTexture(int assetHandle);

    void initBindGroup(bool depthWrite);

    wgpu::BindGroup &bindGroup();

    wgpu::RenderPipeline &renderPipeline();

private:
    wgpu::BindGroup m_bindGroup;

    std::vector<int> m_textureHandles;
};

}
