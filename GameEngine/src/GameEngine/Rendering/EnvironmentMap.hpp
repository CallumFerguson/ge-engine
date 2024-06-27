#pragma once

#include <webgpu/webgpu_cpp.h>
#include "../Assets/Asset.hpp"

namespace GameEngine {

class EnvironmentMap : public Asset {
public:
    explicit EnvironmentMap(const std::string &assetPath);

    int preFilterCubeMapHandle();

    int irradianceCubeMapHandle();

    wgpu::BindGroup &bindGroup();

private:
    int m_preFilterCubeMapHandle;
    int m_irradianceCubeMapHandle;

    wgpu::BindGroup m_bindGroup;
};

}
