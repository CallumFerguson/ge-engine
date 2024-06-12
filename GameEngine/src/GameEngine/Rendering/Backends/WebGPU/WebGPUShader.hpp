#pragma once

#include <string>
#include <webgpu/webgpu_cpp.h>
#include "../../../Assets/Asset.hpp"

namespace GameEngine {

class WebGPUShader : public Asset {
public:
    WebGPUShader() = delete;

    explicit WebGPUShader(const std::string &shaderFilePath);

    wgpu::ShaderModule &shaderModule();

private:
    wgpu::ShaderModule m_shaderModule;
};

}
