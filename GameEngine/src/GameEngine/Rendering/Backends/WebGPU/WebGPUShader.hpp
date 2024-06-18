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

    wgpu::RenderPipeline &renderPipeline(bool depthWrite);

    static void
    registerShaderCreatePipelineFunction(const std::string &shaderUUID, std::function<wgpu::RenderPipeline(const wgpu::ShaderModule &shaderModule, bool depthWrite)> createPipelineFunction);

    static bool shaderHasCreatePipelineFunction(const std::string &shaderUUID);

private:
    wgpu::ShaderModule m_shaderModule;

    wgpu::RenderPipeline m_renderPipelineDepthWrite;
    wgpu::RenderPipeline m_renderPipelineNoDepthWrite;
};

}
