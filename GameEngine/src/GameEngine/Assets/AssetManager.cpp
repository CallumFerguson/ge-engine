#include "AssetManager.hpp"

#include <vector>
#include <unordered_map>

namespace GameEngine {

static std::unordered_map<std::string, int> s_assetPathToHandle;
static std::unordered_map<std::string, int> s_assetUUIDToHandle;

static std::vector<Mesh> s_meshes;
static std::vector<WebGPUShader> s_shaders;

int AssetManager::getOrLoadMeshFromUUID(const std::string &assetUUID) {
    auto it = s_assetUUIDToHandle.find(assetUUID);

    if (it != s_assetUUIDToHandle.end()) {
        return it->second;
    }

    std::string assetPath = "TODO";

    auto &mesh = s_meshes.emplace_back(assetPath);

    int assetHandle = static_cast<int>(s_meshes.size() - 1);
    s_assetPathToHandle[assetPath] = assetHandle;

    s_assetUUIDToHandle[mesh.assetUUID()] = assetHandle;

    return assetHandle;
}

int AssetManager::getOrLoadMeshFromPath(const std::string &assetPath) {
    auto it = s_assetPathToHandle.find(assetPath);

    if (it != s_assetPathToHandle.end()) {
        return it->second;
    }

    auto &mesh = s_meshes.emplace_back(assetPath);

    int assetHandle = static_cast<int>(s_meshes.size() - 1);
    s_assetPathToHandle[assetPath] = assetHandle;

    s_assetUUIDToHandle[mesh.assetUUID()] = assetHandle;

    return assetHandle;
}

Mesh &AssetManager::getMesh(int assetHandle) {
    return s_meshes[assetHandle];
}

int AssetManager::loadShader(const std::string &assetPath) {
    s_shaders.emplace_back(assetPath);
    return static_cast<int>(s_shaders.size() - 1);
}

WebGPUShader &AssetManager::getShader(int assetHandle) {
    return s_shaders[assetHandle];
}

}
