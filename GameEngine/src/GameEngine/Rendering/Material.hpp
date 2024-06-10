#pragma once

#include <string>
#include "../Assets/Asset.hpp"

class Material : public Asset {
public:
    int shaderHandle = -1;

    explicit Material(const std::string &assetPath);

private:
};
