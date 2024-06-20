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

private:
    wgpu::Texture m_texture;

    wgpu::TextureView m_textureView;

    // use pointer so ready can be set after construction even if Texture has moved
    // texture write finish will keep a weak ref to m_ready
    std::shared_ptr<bool> m_ready = std::make_shared<bool>(false);
    std::shared_ptr<std::function<void()>> m_readyCallback = std::make_shared<std::function<void()>>(nullptr);

//    int m_readyStateIndex = -1;
};

}
