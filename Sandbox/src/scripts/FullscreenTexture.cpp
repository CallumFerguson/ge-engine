#include "FullscreenTexture.hpp"

#include <array>
#include <stb_image.h>

#ifdef __EMSCRIPTEN__

#include "GameEngine/Utility/emscriptenUtility.hpp"

#endif

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

    auto device = GameEngine::WebGPURenderer::device();

    m_shaderHandle = GameEngine::AssetManager::getOrLoadAssetFromUUID<GameEngine::WebGPUShader>(shaderUUID);
    auto &shader = GameEngine::AssetManager::getAsset<GameEngine::WebGPUShader>(m_shaderHandle);

    std::array<wgpu::BindGroupEntry, 2> bindGroupEntries;

    bindGroupEntries[0].binding = 0;
    bindGroupEntries[0].sampler = GameEngine::WebGPURenderer::basicSampler();

    wgpu::TextureDescriptor textureDescriptor;
    textureDescriptor.size = {static_cast<uint32_t>(256), static_cast<uint32_t>(256), 1};
    textureDescriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    textureDescriptor.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment;
    auto texture = device.CreateTexture(&textureDescriptor);

#ifdef __EMSCRIPTEN__
//    GameEngine::writeTextureJSAsync(device, texture, "f-texture.png", false, 0);
#else
    int width, height;
    stbi_uc *image = stbi_load("assets/f-texture.png", &width, &height, nullptr, 4);
    if (image == nullptr) {
        GameEngine::exitApp("failed to load image");
    }

    wgpu::ImageCopyTexture destination;
    destination.texture = texture;

    wgpu::TextureDataLayout dataLayout;
    dataLayout.bytesPerRow = width * 4;
    dataLayout.rowsPerImage = height;

    wgpu::Extent3D size = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1};

    device.GetQueue().WriteTexture(&destination, image, width * height * 4, &dataLayout, &size);

    stbi_image_free(image);
#endif

    bindGroupEntries[1].binding = 1;
    bindGroupEntries[1].textureView = texture.CreateView();

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
