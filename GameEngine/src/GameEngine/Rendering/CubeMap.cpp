#include "CubeMap.hpp"

#include "Backends/WebGPU/WebGPURenderer.hpp"

namespace GameEngine {

CubeMap::CubeMap(Texture &equirectangularTexture) {
    std::weak_ptr<wgpu::Texture> cubeMapTextureWeak = m_cubeMapTexture;
    equirectangularTexture.setReadyCallback([cubeMapTextureWeak = std::move(cubeMapTextureWeak)]() {
        auto cubeMapTexture = cubeMapTextureWeak.lock();
        if (!cubeMapTexture) {
            std::cout << "texture doesn't exist!" << std::endl;
            return;
        }

        auto &device = WebGPURenderer::device();

        wgpu::TextureDescriptor textureDescriptor;

        *cubeMapTexture = device.CreateTexture(&textureDescriptor);
    });
}

CubeMap::~CubeMap() {
    std::cout << "bye" << std::endl;
}

}
