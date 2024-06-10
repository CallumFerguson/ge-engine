#pragma once

#include <string>
#include "../Assets/Asset.hpp"

namespace GameEngine {

class Material : public Asset {
public:
    int shaderHandle = -1;

    Material();

    explicit Material(const std::string &assetPath);

private:
};

}
