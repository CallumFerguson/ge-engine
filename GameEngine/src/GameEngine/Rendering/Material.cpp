#include "Material.hpp"

#include <nlohmann/json.hpp>
#include "../Utility/Random.hpp"
#include "../Assets/AssetManager.hpp"
#include "Backends/WebGPU/WebGPURenderer.hpp"

namespace GameEngine {

Material::Material() : Asset(Random::uuid()) {}

Material::Material(const std::string &assetPath) {
    std::ifstream assetFile(assetPath);
    if (!assetFile) {
        std::cerr << "Error: [Material] Could not open file " << assetPath << " for reading!" << std::endl;
        return;
    }

    nlohmann::json materialJSON;
    assetFile >> materialJSON;

    initBindGroup();
}

void Material::initBindGroup() {
    if (shaderHandle == -1) {
        std::cout << "Material::initBindGroup shaderHandle not set" << std::endl;
    }

    auto &shader = AssetManager::getAsset<WebGPUShader>(shaderHandle);

    auto &device = WebGPURenderer::device();

    wgpu::SamplerDescriptor samplerDescriptor;
    samplerDescriptor.magFilter = wgpu::FilterMode::Linear;
    samplerDescriptor.minFilter = wgpu::FilterMode::Linear;
    samplerDescriptor.mipmapFilter = wgpu::MipmapFilterMode::Linear;
    auto sampler = device.CreateSampler(&samplerDescriptor);

    {
        wgpu::BindGroupEntry bindGroupDescriptorEntry0 = {};
        bindGroupDescriptorEntry0.binding = 0;
        bindGroupDescriptorEntry0.buffer = WebGPURenderer::cameraDataBuffer();

        wgpu::BindGroupDescriptor bindGroupDescriptor = {};
        bindGroupDescriptor.layout = shader.renderPipeline().GetBindGroupLayout(0);
        bindGroupDescriptor.entryCount = 1;
        bindGroupDescriptor.entries = &bindGroupDescriptorEntry0;

        m_cameraBindGroup = device.CreateBindGroup(&bindGroupDescriptor);
    }

    {
        std::array<wgpu::BindGroupEntry, 6> bindGroupEntries;
        bindGroupEntries[0].binding = 0;
        bindGroupEntries[0].sampler = sampler;

        int i = 1;
        for (auto &assetHandle: m_textureHandles) {
            auto &texture = AssetManager::getAsset<Texture>(assetHandle);

            bindGroupEntries[i].binding = i;
            bindGroupEntries[i].textureView = texture.texture().CreateView();

            i++;
        }

        wgpu::BindGroupDescriptor bindGroupDescriptor = {};
        bindGroupDescriptor.layout = shader.renderPipeline().GetBindGroupLayout(1);
        bindGroupDescriptor.entryCount = bindGroupEntries.size();
        bindGroupDescriptor.entries = bindGroupEntries.data();

        m_materialBindGroup = device.CreateBindGroup(&bindGroupDescriptor);
    }
}

wgpu::BindGroup &Material::cameraBindGroup() {
    return m_cameraBindGroup;
}

wgpu::BindGroup &Material::materialBindGroup() {
    return m_materialBindGroup;
}

void Material::addTexture(int assetHandle) {
    m_textureHandles.push_back(assetHandle);
}

}
