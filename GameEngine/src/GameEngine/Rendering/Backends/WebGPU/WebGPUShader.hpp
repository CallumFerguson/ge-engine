#pragma once

#include <string>
#include <webgpu/webgpu_cpp.h>

namespace GameEngine {

class WebGPUShader {
public:
    explicit WebGPUShader(const std::string &shaderFilePath);

    wgpu::ShaderModule &shaderModule();

private:
    wgpu::ShaderModule m_shaderModule;
};

}
