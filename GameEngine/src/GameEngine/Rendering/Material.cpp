#include "Material.hpp"

#include <nlohmann/json.hpp>
#include "../Utility/Random.hpp"

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
}

}
