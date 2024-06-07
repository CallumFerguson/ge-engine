#include "AssetManager.hpp"

#include <vector>
#include "../Rendering/Mesh.hpp"

namespace GameEngine {

static std::vector<Mesh> s_meshes;
static std::vector<WebGPUShader> s_shaders;

int AssetManager::loadMesh(const std::string &assetPath) {
    s_meshes.emplace_back(assetPath);
    return static_cast<int>(s_meshes.size() - 1);
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
