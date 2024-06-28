#pragma once

#include <vector>
#include <functional>
#include <memory>
#include <string>
#include <webgpu/webgpu_cpp.h>
#include "../Assets/Asset.hpp"
#include "../Utility/Stream/StreamReader.hpp"

namespace GameEngine {

class Texture : public Asset {
public:
    Texture();

    explicit Texture(const std::string &assetPath, wgpu::TextureFormat requestedFormat = wgpu::TextureFormat::Undefined, bool forceMipLevels = false, wgpu::TextureUsage extraFlags = wgpu::TextureUsage::None);

    wgpu::Texture &texture();

    wgpu::TextureView &cachedTextureView();

    static void writeTextures();

    bool ready();

    void setReadyCallback(std::function<void()> readyCallback);

    static void setTextureReady(int readyStateIndex);

    [[nodiscard]] const wgpu::Extent3D &size() const;

    [[nodiscard]] const uint32_t mipLevelCount();

private:
    wgpu::Texture m_texture;

    wgpu::TextureView m_textureView;

    size_t m_readyStateIndex = -1;

    wgpu::Extent3D m_size;

    uint32_t m_mipLevelCount = 0;
};

}
