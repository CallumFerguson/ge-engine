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

    uint32_t assetVersion = 0;
    assetFile.read(reinterpret_cast<char *>(&assetVersion), sizeof(assetVersion));
    if (assetVersion != 0) {
        std::cout << "unknown material asset version" << std::endl;
    }

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
        std::vector<wgpu::BindGroupEntry> bindGroupEntries(m_textureHandles.size());

        int i = 0;
        for (auto &assetHandle: m_textureHandles) {
            auto &texture = AssetManager::getAsset<Texture>(assetHandle);

            bindGroupEntries[i].binding = i;
            bindGroupEntries[i].textureView = texture.cachedTextureView();

            i++;
        }

        wgpu::BindGroupDescriptor bindGroupDescriptor = {};
        bindGroupDescriptor.layout = WebGPURenderer::pbrMaterialBindGroupLayout();
        bindGroupDescriptor.entryCount = bindGroupEntries.size();
        bindGroupDescriptor.entries = bindGroupEntries.data();

        m_bindGroup = device.CreateBindGroup(&bindGroupDescriptor);
    }
}

wgpu::BindGroup &Material::bindGroup() {
    return m_bindGroup;
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
