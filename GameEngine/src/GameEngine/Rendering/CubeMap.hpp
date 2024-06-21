#pragma once

#include <memory>
#include <webgpu/webgpu_cpp.h>
#include "../Assets/Asset.hpp"
#include "Texture.hpp"

namespace GameEngine {

class CubeMap : public Asset {
public:
    explicit CubeMap(int equirectangularTextureHandle);

private:
    // shared ptr so constructor can have a valid reference after texture ready callback fires
    std::shared_ptr<wgpu::Texture> m_cubeMapTexture = std::make_shared<wgpu::Texture>(nullptr);

    static wgpu::Texture createCubeMapFromEquirectangularTexture(int equirectangularTextureHandle);
};

}
