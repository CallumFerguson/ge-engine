#include "EnvironmentMap.hpp"

#include <nlohmann/json.hpp>
#include "../Assets/AssetManager.hpp"
#include "CubeMap.hpp"
#include "Backends/WebGPU/WebGPURenderer.hpp"
#include "../Utility/Stream/FileStreamWriter.hpp"

namespace GameEngine {

EnvironmentMap::EnvironmentMap(const std::string &assetPath) {
    std::ifstream assetFile(appendAssetFileIfNeeded(assetPath, ".geenvironmentmap"), std::ios::binary);
    if (!assetFile) {
        std::cerr << "Error: [EnvironmentMap] Could not open asset " << assetPath << " for reading!" << std::endl;
        return;
    }

    char uuid[37];
    uuid[36] = '\0';
    assetFile.read(uuid, 36);
    m_assetUUID = uuid;

    uint32_t assetVersion = 0;
    assetFile.read(reinterpret_cast<char *>(&assetVersion), sizeof(assetVersion));
    if (assetVersion != 0) {
        std::cout << "unknown environment map asset version" << std::endl;
    }

    nlohmann::json environmentMapJSON;
    assetFile >> environmentMapJSON;

    std::string preFilterTextureUUID = environmentMapJSON["preFilterTextureUUID"];
    int prefilterTextureHandle = AssetManager::getOrLoadAssetFromUUID<Texture>(preFilterTextureUUID);
    m_preFilterCubeMapHandle = AssetManager::createAsset<CubeMap>(prefilterTextureHandle);

    std::string irradianceTextureUUID = environmentMapJSON["irradianceTextureUUID"];
    int irradianceTextureHandle = AssetManager::getOrLoadAssetFromUUID<Texture>(environmentMapJSON["irradianceTextureUUID"]);
    m_irradianceCubeMapHandle = AssetManager::createAsset<CubeMap>(irradianceTextureHandle);

    std::array<wgpu::BindGroupEntry, 4> bindGroupEntries;

    bindGroupEntries[0].binding = 0;
    bindGroupEntries[0].sampler = WebGPURenderer::basicSampler();

    int brdfHandle = AssetManager::getOrLoadAssetFromUUID<Texture>(BRDF_UUID);
    auto &brdf = AssetManager::getAsset<Texture>(brdfHandle);
    bindGroupEntries[1].binding = 1;
    bindGroupEntries[1].textureView = brdf.cachedTextureView();

    auto &preFilterCubeMap = AssetManager::getAsset<CubeMap>(m_preFilterCubeMapHandle);
    bindGroupEntries[2].binding = 2;
    bindGroupEntries[2].textureView = preFilterCubeMap.cachedTextureView();

    auto &irradianceCubeMap = AssetManager::getAsset<CubeMap>(m_irradianceCubeMapHandle);
    bindGroupEntries[3].binding = 3;
    bindGroupEntries[3].textureView = irradianceCubeMap.cachedTextureView();

    wgpu::BindGroupDescriptor bindGroupDescriptor;
    bindGroupDescriptor.layout = WebGPURenderer::pbrEnvironmentMapBindGroupLayout();
    bindGroupDescriptor.entryCount = bindGroupEntries.size();
    bindGroupDescriptor.entries = bindGroupEntries.data();

    m_bindGroup = WebGPURenderer::device().CreateBindGroup(&bindGroupDescriptor);
}

int EnvironmentMap::preFilterCubeMapHandle() {
    return m_preFilterCubeMapHandle;
}

int EnvironmentMap::irradianceCubeMapHandle() {
    return m_irradianceCubeMapHandle;
}

wgpu::BindGroup &EnvironmentMap::bindGroup() {
    return m_bindGroup;
}

}
