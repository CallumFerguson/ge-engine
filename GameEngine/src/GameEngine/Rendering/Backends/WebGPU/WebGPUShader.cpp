#include "WebGPUShader.hpp"

#include <functional>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include "WebGPURenderer.hpp"
#include "../../../Core/Exit.hpp"
#include "../../../Utility/utility.hpp"

namespace GameEngine {

static std::unordered_map<std::string, std::function<wgpu::RenderPipeline(const wgpu::ShaderModule &shaderModule, bool depthWrite)>> s_shaderUUIDToCreatePipelineFunction;

void
WebGPUShader::registerShaderCreatePipelineFunction(const std::string &shaderUUID, std::function<wgpu::RenderPipeline(const wgpu::ShaderModule &shaderModule, bool depthWrite)> createPipelineFunction) {
    if (s_shaderUUIDToCreatePipelineFunction.contains(shaderUUID)) {
        std::cout << "registerShaderCreatePipelineFunction shader with uuid " << shaderUUID << " already has a function." << std::endl;
        return;
    }
    s_shaderUUIDToCreatePipelineFunction[shaderUUID] = std::move(createPipelineFunction);
}

bool WebGPUShader::shaderHasCreatePipelineFunction(const std::string &shaderUUID) {
    return s_shaderUUIDToCreatePipelineFunction.contains(shaderUUID);
}

WebGPUShader::WebGPUShader(const std::string &shaderFilePath) {
    auto &device = WebGPURenderer::device();

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

    if (!s_shaderUUIDToCreatePipelineFunction.contains(m_assetUUID)) {
        std::cout << "Missing createPipelineFunction for shader: " << shaderFilePath << " : " << m_assetUUID << std::endl;
        std::cout << "register one using WebGPUShader::registerShaderCreatePipelineFunction" << std::endl;
        return;
    }

    m_renderPipelineDepthWrite = s_shaderUUIDToCreatePipelineFunction[m_assetUUID](m_shaderModule, true);
    m_renderPipelineNoDepthWrite = s_shaderUUIDToCreatePipelineFunction[m_assetUUID](m_shaderModule, false);
}

wgpu::ShaderModule &WebGPUShader::shaderModule() {
    return m_shaderModule;
}

wgpu::RenderPipeline &WebGPUShader::renderPipeline(bool depthWrite) {
    return depthWrite ? m_renderPipelineDepthWrite : m_renderPipelineNoDepthWrite;
}

}
