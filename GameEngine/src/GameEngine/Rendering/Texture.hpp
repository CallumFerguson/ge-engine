#pragma once

#include <string>
#include <webgpu/webgpu_cpp.h>
#include "../Assets/Asset.hpp"

namespace GameEngine {

class Texture : public Asset {
public:
    Texture();

    explicit Texture(const std::string &assetPath);

    wgpu::Texture &texture();

    wgpu::TextureView &cachedTextureView();

    static void writeTextures();

private:
    wgpu::Texture m_texture;

    wgpu::TextureView m_textureView;
};

}
