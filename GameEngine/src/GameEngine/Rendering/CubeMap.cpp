#include "CubeMap.hpp"

#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Backends/WebGPU/WebGPURenderer.hpp"
#include "../Assets/AssetManager.hpp"

namespace GameEngine {

static const int cubeMapFacePixelLength = 2048;

CubeMap::CubeMap(int equirectangularTextureHandle) {
    auto &device = WebGPURenderer::device();

    wgpu::TextureDescriptor textureDescriptor;
    textureDescriptor.size = {cubeMapFacePixelLength, cubeMapFacePixelLength, 6};
    textureDescriptor.format = wgpu::TextureFormat::RGBA16Float;
    textureDescriptor.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding;
    *m_cubeMapTexture = device.CreateTexture(&textureDescriptor);

    std::weak_ptr<wgpu::Texture> cubeMapTextureWeak = m_cubeMapTexture;
    AssetManager::getAsset<Texture>(equirectangularTextureHandle).setReadyCallback([
                                                                                           cubeMapTextureWeak = std::move(cubeMapTextureWeak),
                                                                                           equirectangularTextureHandle]() {
        auto cubeMapTexture = cubeMapTextureWeak.lock();
        if (!cubeMapTexture) {
            return;
        }

        writeCubeMapFromEquirectangularTexture(equirectangularTextureHandle, *cubeMapTexture);
    });
}

void CubeMap::writeCubeMapFromEquirectangularTexture(int equirectangularTextureHandle, wgpu::Texture& cubeMapTexture) {
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

    int shaderHandle = GameEngine::AssetManager::getOrLoadAssetFromUUID<GameEngine::WebGPUShader>(shaderUUID);
    auto &shader = GameEngine::AssetManager::getAsset<GameEngine::WebGPUShader>(shaderHandle);

    glm::mat4 mat(1.0f);

    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.mappedAtCreation = true;
    bufferDescriptor.size = 64;
    bufferDescriptor.usage = wgpu::BufferUsage::Uniform;
    auto viewDirectionProjectionInverseBuffer = device.CreateBuffer(&bufferDescriptor);
    auto mappedData = viewDirectionProjectionInverseBuffer.GetMappedRange();
    std::memcpy(mappedData, glm::value_ptr(mat), 64);
    viewDirectionProjectionInverseBuffer.Unmap();

    std::array<wgpu::BindGroupEntry, 3> entries;
    entries[0].binding = 0;
    entries[0].buffer = viewDirectionProjectionInverseBuffer;

    entries[1].binding = 1;
    entries[1].sampler = WebGPURenderer::basicSampler();

    auto& equirectangularTexture = AssetManager::getAsset<Texture>(equirectangularTextureHandle);
    entries[2].binding = 2;
    entries[2].textureView = equirectangularTexture.cachedTextureView();

    wgpu::BindGroupDescriptor bindGroupDescriptor;
    bindGroupDescriptor.layout = shader.renderPipeline(false).GetBindGroupLayout(0);
    bindGroupDescriptor.entryCount = entries.size();
    bindGroupDescriptor.entries = entries.data();

    auto bindGroup = device.CreateBindGroup(&bindGroupDescriptor);

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
    renderPassEncoder.SetBindGroup(0, bindGroup);
    renderPassEncoder.Draw(3);
    renderPassEncoder.End();

    auto commandBuffer = encoder.Finish();
    device.GetQueue().Submit(1, &commandBuffer);
}

wgpu::Texture &CubeMap::texture() {
    return *m_cubeMapTexture;
}

wgpu::TextureView &CubeMap::cachedTextureView() {
    if (!m_textureView) {
        wgpu::TextureViewDescriptor textureViewDescriptor;
        textureViewDescriptor.dimension = wgpu::TextureViewDimension::Cube;
        m_textureView = m_cubeMapTexture->CreateView(&textureViewDescriptor);
    }
    return m_textureView;
}

}
