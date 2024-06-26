#include "EnvironmentMap.hpp"

#include <vector>
#include "../Assets/AssetManager.hpp"
#include "CubeMap.hpp"

namespace GameEngine {

EnvironmentMap::EnvironmentMap(const std::string &assetPath) {
    std::ifstream assetFile(assetPath, std::ios::binary);
    if (!assetFile) {
        std::cerr << "Error: [EnvironmentMap] Could not open file " << assetPath << " for reading!" << std::endl;
        return;
    }

    char uuid[37];
    uuid[36] = '\0';
    assetFile.read(uuid, 36);
    m_assetUUID = uuid;

    uint32_t preFilterByteSize;
    assetFile.read(reinterpret_cast<char *>(&preFilterByteSize), sizeof(uint32_t));
    std::vector<char> preFilterFileData(preFilterByteSize);
    assetFile.read(preFilterFileData.data(), preFilterByteSize);

    int prefilterTextureHandle = AssetManager::createAsset<Texture>(std::move(preFilterFileData), ".getexture");
    m_preFilterCubeMapHandle = AssetManager::createAsset<CubeMap>(prefilterTextureHandle);

    uint32_t irradianceByteSize;
    assetFile.read(reinterpret_cast<char *>(&irradianceByteSize), sizeof(uint32_t));
    std::vector<char> irradianceFileData(irradianceByteSize);
    assetFile.read(irradianceFileData.data(), irradianceByteSize);

    int irradianceTextureHandle = AssetManager::createAsset<Texture>(std::move(irradianceFileData), ".getexture");
    m_irradianceCubeMapHandle = AssetManager::createAsset<CubeMap>(irradianceTextureHandle);
}

int EnvironmentMap::skyboxCubeMapHandle() {
    return m_preFilterCubeMapHandle;
}

int EnvironmentMap::preFilterCubeMapHandle() {
    return m_preFilterCubeMapHandle;
}

int EnvironmentMap::irradianceCubeMapHandle() {
    return m_irradianceCubeMapHandle;
}

}
