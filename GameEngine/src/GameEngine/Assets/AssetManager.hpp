#pragma once

#include <string>

namespace GameEngine {

class AssetManager {
public:
    static int loadMesh(const std::string &assetPath);
};

}
