#include "AssetManager.hpp"

#include <vector>
#include "../Rendering/Mesh.hpp"

namespace GameEngine {

static std::vector<Mesh> s_meshes;

int AssetManager::loadMesh(const std::string &assetPath) {
    s_meshes.emplace_back(assetPath);
    return static_cast<int>(s_meshes.size() - 1);
}

}
