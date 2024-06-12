#include "WebGPUShader.hpp"

#include <fstream>
#include <sstream>
#include "WebGPURenderer.hpp"
#include "../../../Core/Exit.hpp"
#include "../../../Utility/utility.hpp"

namespace GameEngine {

WebGPUShader::WebGPUShader(const std::string &shaderFilePath) {
    auto device = WebGPURenderer::device();

    std::ifstream shaderFile(shaderFilePath, std::ios::binary);
    if (!shaderFile) {
        exitApp("Could not open shader file: " + shaderFilePath);
    }
    std::stringstream shaderBuffer;
    shaderBuffer << shaderFile.rdbuf();
    auto shaderString = shaderBuffer.str();

    m_assetUUID = shaderString.substr(2, 36);
    if (!isUUID(m_assetUUID)) {
        std::cout << "shader missing uuid. first line of file should be \"//aaaaaaaa-aaaa-aaaa-aaaa-aaaaaaaaaaaa\"" << std::endl;
        m_assetUUID.clear();
    }

    wgpu::ShaderModuleWGSLDescriptor wgslDescriptor{};
    wgslDescriptor.code = shaderString.c_str();

    wgpu::ShaderModuleDescriptor shaderModuleDescriptor = {};
    shaderModuleDescriptor.nextInChain = &wgslDescriptor;

    m_shaderModule = device.CreateShaderModule(&shaderModuleDescriptor);
}

wgpu::ShaderModule &WebGPUShader::shaderModule() {
    return m_shaderModule;
}

}
