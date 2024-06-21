#include "CubeMap.hpp"

#include "Backends/WebGPU/WebGPURenderer.hpp"
#include "../Assets/AssetManager.hpp"

namespace GameEngine {

static const int cubeMapFacePixelLength = 2048;

CubeMap::CubeMap(int equirectangularTextureHandle) {
    std::weak_ptr<wgpu::Texture> cubeMapTextureWeak = m_cubeMapTexture;
    AssetManager::getAsset<Texture>(equirectangularTextureHandle).setReadyCallback([
                                                                                           cubeMapTextureWeak = std::move(cubeMapTextureWeak),
                                                                                           equirectangularTextureHandle]() {
        auto cubeMapTexture = cubeMapTextureWeak.lock();
        if (!cubeMapTexture) {
            return;
        }

        *cubeMapTexture = createCubeMapFromEquirectangularTexture(equirectangularTextureHandle);
    });
}

wgpu::Texture CubeMap::createCubeMapFromEquirectangularTexture(int equirectangularTextureHandle) {
    auto &device = WebGPURenderer::device();

    const auto shaderUUID = EQUIRECTANGULAR_SKYBOX_SHADER_UUID;
    if(!GameEngine::WebGPUShader::shaderHasCreatePipelineFunction(shaderUUID)) {
        GameEngine::WebGPUShader::registerShaderCreatePipelineFunction(shaderUUID, [](const wgpu::ShaderModule &shaderModule, bool depthWrite) {
            auto device = GameEngine::WebGPURenderer::device();

            wgpu::RenderPipelineDescriptor pipelineDescriptor = {};

            pipelineDescriptor.layout = nullptr; // auto layout

            wgpu::ColorTargetState colorTargetState = {};
            colorTargetState.format = wgpu::TextureFormat::RGBA16Float;

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

            pipelineDescriptor.multisample.count = 1;

            return device.CreateRenderPipeline(&pipelineDescriptor);
        });
    }

    wgpu::TextureDescriptor textureDescriptor;
    textureDescriptor.size = {cubeMapFacePixelLength, cubeMapFacePixelLength, 6};
    textureDescriptor.format = wgpu::TextureFormat::RGBA16Float;
    textureDescriptor.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding;
    auto cubeMapTexture = device.CreateTexture(&textureDescriptor);

    int shaderHandle = GameEngine::AssetManager::getOrLoadAssetFromUUID<GameEngine::WebGPUShader>(shaderUUID);
    auto &shader = GameEngine::AssetManager::getAsset<GameEngine::WebGPUShader>(shaderHandle);

    wgpu::TextureViewDescriptor textureViewDescriptor;
    textureViewDescriptor.dimension = wgpu::TextureViewDimension::e2D;
    textureViewDescriptor.baseArrayLayer = 0;
    textureViewDescriptor.arrayLayerCount = 1;
    textureViewDescriptor.baseMipLevel = 0;
    textureViewDescriptor.mipLevelCount = 1;

    wgpu::RenderPassColorAttachment colorAttachment;
    colorAttachment.view = cubeMapTexture.CreateView(&textureViewDescriptor);
    colorAttachment.loadOp = wgpu::LoadOp::Clear;
    colorAttachment.storeOp = wgpu::StoreOp::Store;
    colorAttachment.clearValue = wgpu::Color{0, 0, 0, 1};

    wgpu::RenderPassDescriptor renderPassDescriptor;
    renderPassDescriptor.colorAttachmentCount = 1;
    renderPassDescriptor.colorAttachments = &colorAttachment;

    auto encoder = device.CreateCommandEncoder();
    auto renderPassEncoder = encoder.BeginRenderPass(&renderPassDescriptor);
    renderPassEncoder.SetPipeline(shader.renderPipeline(false));
    renderPassEncoder.Draw(3);
    auto commandBuffer = encoder.Finish();
    device.GetQueue().Submit(1, &commandBuffer);

    return cubeMapTexture;
}

}
