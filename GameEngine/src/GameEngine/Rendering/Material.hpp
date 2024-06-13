#pragma once

#include <vector>
#include <string>
#include <webgpu/webgpu_cpp.h>
#include "../Assets/Asset.hpp"

namespace GameEngine {

class Material : public Asset {
public:
    int shaderHandle = -1;

    Material();

    explicit Material(const std::string &assetPath);

    void addTexture(int assetHandle);

    void initBindGroup();

    wgpu::BindGroup &cameraBindGroup();

    wgpu::BindGroup &materialBindGroup();

private:
    wgpu::BindGroup m_cameraBindGroup;
    wgpu::BindGroup m_materialBindGroup;

    std::vector<int> m_textureHandles;
};

}
