#include "Material.hpp"

#include <nlohmann/json.hpp>
#include "../Utility/Random.hpp"
#include "../Assets/AssetManager.hpp"
#include "Backends/WebGPU/WebGPURenderer.hpp"
#include "EnvironmentMap.hpp"
#include "CubeMap.hpp"

namespace GameEngine {

Material::Material() : Asset(Random::uuid()) {}

Material::Material(const std::string &assetPath) {
    std::ifstream assetFile(assetPath);
    if (!assetFile) {
        std::cerr << "Error: [Material] Could not open file " << assetPath << " for reading!" << std::endl;
        return;
    }

    char uuid[37];
    uuid[36] = '\0';
    assetFile.read(uuid, 36);
    m_assetUUID = uuid;

    nlohmann::json materialJSON;
    assetFile >> materialJSON;

    shaderHandle = AssetManager::getOrLoadAssetFromUUID<WebGPUShader>(materialJSON["shader"]["uuid"]);

    for (auto &textureUUID: materialJSON["textureUUIDs"]) {
        auto textureUUIDString = textureUUID.get<std::string>();
        m_textureHandles.push_back(AssetManager::getOrLoadAssetFromUUID<Texture>(textureUUIDString));
    }

    renderQueue = materialJSON["renderQueue"];

    initBindGroup(true);
    initBindGroup(false);
}

void Material::initBindGroup(bool depthWrite) {
    if (shaderHandle == -1) {
        std::cout << "Material::initBindGroup shaderHandle not set" << std::endl;
    }

    auto &shader = AssetManager::getAsset<WebGPUShader>(shaderHandle);

    auto &device = WebGPURenderer::device();

    {
        wgpu::BindGroupEntry bindGroupDescriptorEntry0 = {};
        bindGroupDescriptorEntry0.binding = 0;
        bindGroupDescriptorEntry0.buffer = WebGPURenderer::cameraDataBuffer();

        wgpu::BindGroupDescriptor bindGroupDescriptor = {};
        bindGroupDescriptor.layout = shader.renderPipeline(depthWrite).GetBindGroupLayout(0);
        bindGroupDescriptor.entryCount = 1;
        bindGroupDescriptor.entries = &bindGroupDescriptorEntry0;

        if (depthWrite) {
            m_cameraBindGroupDepthWrite = device.CreateBindGroup(&bindGroupDescriptor);
        } else {
            m_cameraBindGroupNoDepthWrite = device.CreateBindGroup(&bindGroupDescriptor);
        }
    }

    {
        std::vector<wgpu::BindGroupEntry> bindGroupEntries(m_textureHandles.size() + 4);

        bindGroupEntries[0].binding = 0;
        bindGroupEntries[0].sampler = WebGPURenderer::basicSampler();

        int brdfTextureHandle = AssetManager::getOrLoadAssetFromUUID<Texture>(BRDF_UUID);
        auto &brdfTexture = AssetManager::getAsset<Texture>(brdfTextureHandle);

        bindGroupEntries[1].binding = 1;
        bindGroupEntries[1].textureView = brdfTexture.cachedTextureView();

        int i = 2;
        for (auto &assetHandle: m_textureHandles) {
            auto &texture = AssetManager::getAsset<Texture>(assetHandle);

            bindGroupEntries[i].binding = i;
            bindGroupEntries[i].textureView = texture.cachedTextureView();

            i++;
        }

        auto environmentMap = AssetManager::getAsset<EnvironmentMap>(0);
        auto preFilterCubeMap = AssetManager::getAsset<CubeMap>(environmentMap.preFilterCubeMapHandle());
        auto irradianceCubeMap = AssetManager::getAsset<CubeMap>(environmentMap.irradianceCubeMapHandle());

        bindGroupEntries[i].binding = i;
        bindGroupEntries[i].textureView = preFilterCubeMap.cachedTextureView();
        i++;

        bindGroupEntries[i].binding = i;
        bindGroupEntries[i].textureView = irradianceCubeMap.cachedTextureView();
        i++;

        wgpu::BindGroupDescriptor bindGroupDescriptor = {};
        bindGroupDescriptor.layout = shader.renderPipeline(depthWrite).GetBindGroupLayout(1);
        bindGroupDescriptor.entryCount = bindGroupEntries.size();
        bindGroupDescriptor.entries = bindGroupEntries.data();

        if (depthWrite) {
            m_materialBindGroupDepthWrite = device.CreateBindGroup(&bindGroupDescriptor);
        } else {
            m_materialBindGroupNoDepthWrite = device.CreateBindGroup(&bindGroupDescriptor);
        }
    }
}

wgpu::BindGroup &Material::cameraBindGroup() {
    switch (renderQueue) {
        case RenderQueue::Opaque:
            return m_cameraBindGroupDepthWrite;
        case RenderQueue::Transparent:
            return m_cameraBindGroupNoDepthWrite;
        default:
            std::cout << "material cameraBindGroup unknown render queue: " << static_cast<uint8_t>(renderQueue) << std::endl;
            return m_cameraBindGroupDepthWrite;
    }
}

wgpu::BindGroup &Material::materialBindGroup() {
    switch (renderQueue) {
        case RenderQueue::Opaque:
            return m_materialBindGroupDepthWrite;
        case RenderQueue::Transparent:
            return m_materialBindGroupNoDepthWrite;
        default:
            std::cout << "material materialBindGroup unknown render queue: " << static_cast<uint8_t>(renderQueue) << std::endl;
            return m_materialBindGroupDepthWrite;
    }
}

void Material::addTexture(int assetHandle) {
    m_textureHandles.push_back(assetHandle);
}

wgpu::RenderPipeline &Material::renderPipeline() {
    auto &shader = AssetManager::getAsset<WebGPUShader>(shaderHandle);
    switch (renderQueue) {
        case RenderQueue::Opaque:
            return shader.renderPipeline(true);
        case RenderQueue::Transparent:
            return shader.renderPipeline(false);
        default:
            std::cout << "material renderPipeline unknown render queue: " << static_cast<uint8_t>(renderQueue) << std::endl;
            return shader.renderPipeline(true);
    }
}

}
