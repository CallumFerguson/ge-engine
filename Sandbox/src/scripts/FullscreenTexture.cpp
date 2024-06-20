#include "FullscreenTexture.hpp"

#include <array>
#include <stb_image.h>

void FullscreenTexture::onStart() {
    const auto shaderUUID = "03e59a18-0eed-4892-af7a-99c36782b368";
    if(!GameEngine::WebGPUShader::shaderHasCreatePipelineFunction(shaderUUID)) {
        GameEngine::WebGPUShader::registerShaderCreatePipelineFunction(shaderUUID, [](const wgpu::ShaderModule &shaderModule, bool depthWrite) {
            auto device = GameEngine::WebGPURenderer::device();

            wgpu::RenderPipelineDescriptor pipelineDescriptor = {};

            pipelineDescriptor.layout = nullptr; // auto layout

            wgpu::ColorTargetState colorTargetState = {};
            colorTargetState.format = GameEngine::WebGPURenderer::mainSurfacePreferredFormat();

            wgpu::FragmentState fragment = {};
            fragment.module = shaderModule;
            fragment.entryPoint = "frag";
            fragment.targetCount = 1;
            fragment.targets = &colorTargetState;

            wgpu::VertexState vertex = {};
            vertex.module = shaderModule;
            vertex.entryPoint = "vert";
            vertex.bufferCount = 0;

            pipelineDescriptor.vertex = vertex;
            pipelineDescriptor.fragment = &fragment;

            pipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
            pipelineDescriptor.primitive.cullMode = wgpu::CullMode::Back;

            pipelineDescriptor.multisample.count = GameEngine::WebGPURenderer::multisampleCount;

            wgpu::DepthStencilState depthStencilState = {};
            depthStencilState.depthWriteEnabled = depthWrite;
            depthStencilState.depthCompare = wgpu::CompareFunction::Less;
            depthStencilState.format = wgpu::TextureFormat::Depth24Plus;

            pipelineDescriptor.depthStencil = &depthStencilState;

            return device.CreateRenderPipeline(&pipelineDescriptor);
        });
    }

    int textureHandle = GameEngine::AssetManager::getOrLoadAssetFromPath<GameEngine::Texture>("assets/test.hdr");
    auto& texture = GameEngine::AssetManager::getAsset<GameEngine::Texture>(textureHandle);

//    int brdfTextureHandle = GameEngine::AssetManager::getOrLoadAssetFromUUID<GameEngine::Texture>(BRDF_UUID);
//    auto& brdfTexture = GameEngine::AssetManager::getAsset<GameEngine::Texture>(brdfTextureHandle);

    auto device = GameEngine::WebGPURenderer::device();

    m_shaderHandle = GameEngine::AssetManager::getOrLoadAssetFromUUID<GameEngine::WebGPUShader>(shaderUUID);
    auto &shader = GameEngine::AssetManager::getAsset<GameEngine::WebGPUShader>(m_shaderHandle);

    std::array<wgpu::BindGroupEntry, 2> bindGroupEntries;

    bindGroupEntries[0].binding = 0;
    bindGroupEntries[0].sampler = GameEngine::WebGPURenderer::basicSampler();

    bindGroupEntries[1].binding = 1;
    bindGroupEntries[1].textureView = texture.cachedTextureView();

    wgpu::BindGroupDescriptor bindGroupDescriptor = {};
    bindGroupDescriptor.layout = shader.renderPipeline(true).GetBindGroupLayout(0);
    bindGroupDescriptor.entryCount = bindGroupEntries.size();
    bindGroupDescriptor.entries = bindGroupEntries.data();

    m_bindGroup = device.CreateBindGroup(&bindGroupDescriptor);
}

void FullscreenTexture::onMainRenderPass() {
    auto& shader = GameEngine::AssetManager::getAsset<GameEngine::WebGPUShader>(m_shaderHandle);

    auto renderPassEncoder = GameEngine::WebGPURenderer::renderPassEncoder();
    renderPassEncoder.SetPipeline(shader.renderPipeline(true));

    renderPassEncoder.SetBindGroup(0, m_bindGroup);

    renderPassEncoder.Draw(3);
}
