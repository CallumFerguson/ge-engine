#pragma once

#include <vector>
#include <functional>
#include <memory>
#include <string>
#include <webgpu/webgpu_cpp.h>
#include "../Assets/Asset.hpp"
#include "../Utility/Stream/StreamReader.hpp"
#include "../Utility/Stream/StreamWriter.hpp"

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

//    static void getTextureData(const wgpu::Texture &texture, wgpu::TextureFormat textureFormat, const wgpu::Extent3D &size, uint32_t mipLevel, std::function<void(const float *imageData)> &&dataReadyCallback);

private:
    wgpu::Texture m_texture;
    wgpu::TextureView m_textureView;

    wgpu::Extent3D m_size;
    uint32_t m_mipLevelCount = 0;
    wgpu::TextureFormat m_textureFormat = wgpu::TextureFormat::Undefined;

    size_t m_readyStateIndex = -1;
};

}
