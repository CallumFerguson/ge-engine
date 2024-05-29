#include "WebGPUShader.hpp"

#include <fstream>
#include <sstream>
#include "WebGPURenderer.hpp"

WebGPUShader::WebGPUShader(const std::string &shaderFilePath) {
    auto device = WebGPURenderer::device();

    std::ifstream shaderFile(shaderFilePath, std::ios::binary);
    if (!shaderFile) {
        throw std::runtime_error("Could not open shader file: " + shaderFilePath);
    }
    std::stringstream shaderBuffer;
    shaderBuffer << shaderFile.rdbuf();
    auto shaderString = shaderBuffer.str();

    wgpu::ShaderModuleWGSLDescriptor wgslDescriptor{};
    wgslDescriptor.code = shaderString.c_str();

    wgpu::ShaderModuleDescriptor shaderModuleDescriptor = {};
    shaderModuleDescriptor.nextInChain = &wgslDescriptor;

    m_shaderModule = device.CreateShaderModule(&shaderModuleDescriptor);
}

wgpu::ShaderModule &WebGPUShader::shaderModule() {
    return m_shaderModule;
}
