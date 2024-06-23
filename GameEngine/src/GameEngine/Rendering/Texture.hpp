#pragma once

#include <functional>
#include <memory>
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

    bool ready();

    void setReadyCallback(std::function<void()> readyCallback);

    static void setTextureReady(int readyStateIndex);

    [[nodiscard]] const wgpu::Extent3D &size() const;

private:
    wgpu::Texture m_texture;

    wgpu::TextureView m_textureView;

    size_t m_readyStateIndex = -1;

    wgpu::Extent3D m_size;
};

}
