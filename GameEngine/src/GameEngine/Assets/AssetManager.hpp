#pragma once

#include <string>
#include "../Rendering/Mesh.hpp"
#include "../Rendering/Material.hpp"
#include "../Rendering/Texture.hpp"
#include "../Rendering/Backends/WebGPU/WebGPUShader.hpp"

namespace GameEngine {

class AssetManager {
public:
    static void registerAssetUUIDs();

    template<typename T>
    static int getOrLoadAssetFromUUID(const std::string &assetUUID);

    template<typename T>
    static int getOrLoadAssetFromPath(const std::string &assetPath);

    template<typename T>
    static T &getAsset(int assetHandle);

    template<typename T>
    static int createAsset(T asset);

private:
    template<typename T>
    struct AssetData {
        static std::vector<T> assets;
    };
};

#include "AssetManager.tpp"

}
