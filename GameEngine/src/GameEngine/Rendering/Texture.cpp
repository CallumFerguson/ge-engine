#include "Texture.hpp"

#include <nlohmann/json.hpp>
#include "../Utility/Random.hpp"

namespace GameEngine {

Texture::Texture() : Asset(Random::uuid()) {}

Texture::Texture(const std::string &assetPath) {
    std::ifstream assetFile(assetPath);
    if (!assetFile) {
        std::cerr << "Error: [Texture] Could not open file " << assetPath << " for reading!" << std::endl;
        return;
    }

    nlohmann::json materialJSON;
    assetFile >> materialJSON;
}

}
