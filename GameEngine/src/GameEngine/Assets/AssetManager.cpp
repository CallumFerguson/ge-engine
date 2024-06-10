#include "AssetManager.hpp"

#include <iostream>
#include <vector>
#include <unordered_map>

namespace GameEngine {

static std::unordered_map<std::string, int> s_assetPathToHandle;
static std::unordered_map<std::string, int> s_assetUUIDToHandle;

static std::vector<Mesh> s_meshes;
static std::vector<WebGPUShader> s_shaders;
static std::vector<Material> s_materials;
static std::vector<Texture> s_textures;

static std::unordered_map<std::string, std::string> s_assetUUIDToPath;

void AssetManager::registerAssetUUIDs() {
    for (const auto &entry: std::filesystem::recursive_directory_iterator("assets")) {
        if (entry.is_regular_file() && entry.path().extension() == ".gemesh") {
            std::ifstream inputFile(entry.path(), std::ios::binary);
            if (!inputFile) {
                std::cerr << "Error: assetUUIDToPath could not open file " << entry.path() << " for reading!" << std::endl;
                break;
            }

            char uuid[37];
            uuid[36] = '\0';
            inputFile.read(uuid, 36);
            s_assetUUIDToPath[uuid] = entry.path().string();
        }
    }
}

int AssetManager::getOrLoadMeshFromUUID(const std::string &assetUUID) {
    auto it = s_assetUUIDToHandle.find(assetUUID);

    if (it != s_assetUUIDToHandle.end()) {
        return it->second;
    }

    auto it2 = s_assetUUIDToPath.find(assetUUID);

    if (it2 == s_assetUUIDToPath.end()) {
        std::cout << "could not find asset with uuid: " << assetUUID << std::endl;
        return -1;
    }

    std::string &assetPath = it2->second;

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

int AssetManager::createMesh(Mesh mesh) {
    auto &meshRef = s_meshes.emplace_back(std::move(mesh));

    int assetHandle = static_cast<int>(s_meshes.size() - 1);

    s_assetUUIDToHandle[meshRef.assetUUID()] = assetHandle;

    return assetHandle;
}

int AssetManager::createMaterial(Material material) {
    auto &materialRef = s_materials.emplace_back(std::move(material));

    int assetHandle = static_cast<int>(s_materials.size() - 1);

    s_assetUUIDToHandle[materialRef.assetUUID()] = assetHandle;

    return assetHandle;
}

int AssetManager::createTexture(Texture texture) {
    auto &textureRef = s_textures.emplace_back(std::move(texture));

    int assetHandle = static_cast<int>(s_textures.size() - 1);

    s_assetUUIDToHandle[textureRef.assetUUID()] = assetHandle;

    return assetHandle;
}

int AssetManager::loadShader(const std::string &assetPath) {
    s_shaders.emplace_back(assetPath);
    return static_cast<int>(s_shaders.size() - 1);
}

WebGPUShader &AssetManager::getShader(int assetHandle) {
    return s_shaders[assetHandle];
}

}
