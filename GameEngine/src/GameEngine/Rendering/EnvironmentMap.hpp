#pragma once

#include "../Assets/Asset.hpp"

namespace GameEngine {

class EnvironmentMap : public Asset {
public:
    explicit EnvironmentMap(const std::string &assetPath);

    int skyboxCubeMapHandle();

    int preFilterCubeMapHandle();

    int irradianceCubeMapHandle();

private:
    int m_preFilterCubeMapHandle;
    int m_irradianceCubeMapHandle;
};

}
