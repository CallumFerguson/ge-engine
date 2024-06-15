#include "Texture.hpp"

#include <vector>
#include <stb_image.h>
#include "../Utility/Random.hpp"
#include "Backends/WebGPU/WebGPURenderer.hpp"
#include "../Utility/TimingHelper.hpp"
#include "Backends/WebGPU/generateMipmapWebGPU.hpp"

#ifdef __EMSCRIPTEN__

#include "../Utility/emscriptenUtility.hpp"

#else

#include <mutex>
#include "../Utility/ThreadPool.hpp"

#endif

namespace GameEngine {

const int channels = 4;

#ifndef __EMSCRIPTEN__

struct ImageResult {
    stbi_uc *image;
    int width;
    int height;
    wgpu::Texture texture;
};

static std::mutex s_stbiImagesMutex;
static std::vector<ImageResult> s_imageResults;
#endif

Texture::Texture() : Asset(Random::uuid()) {}

Texture::Texture(const std::string &assetPath) {
    std::ifstream assetFile(assetPath, std::ios::binary | std::ios::ate);
    if (!assetFile) {
        std::cerr << "Error: [Texture] Could not open file " << assetPath << " for reading!" << std::endl;
        return;
    }

    auto imageByteLength = static_cast<std::streamsize>(assetFile.tellg()) - 36;
    assetFile.seekg(0, std::ios::beg);

    char uuid[37];
    uuid[36] = '\0';
    assetFile.read(uuid, 36);
    m_assetUUID = uuid;

    std::vector<uint8_t> imageData(imageByteLength);
    assetFile.read(reinterpret_cast<char *>(imageData.data()), imageByteLength);

    int width, height;
    if (!stbi_info_from_memory(imageData.data(), static_cast<int>(imageData.size()), &width, &height, nullptr)) {
        std::cerr << "Failed to get image info from memory" << std::endl;
        return;
    }

    auto &device = WebGPURenderer::device();

    wgpu::TextureDescriptor textureDescriptor;
    textureDescriptor.size = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1};
    textureDescriptor.mipLevelCount = numMipLevels(textureDescriptor.size);
    textureDescriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    textureDescriptor.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment;
    m_texture = device.CreateTexture(&textureDescriptor);

#ifdef __EMSCRIPTEN__
    writeTextureJSAsync(device, m_texture, imageData);
#else
    ThreadPool::instance().queueJob([imageData = std::move(imageData), texture = m_texture]() mutable {
        int width, height;
        unsigned char *image = stbi_load_from_memory(imageData.data(), static_cast<int>(imageData.size()), &width, &height, nullptr, channels);
        if (image == nullptr) {
            std::cerr << "Failed to load image from memory" << std::endl;
            return;
        }

        {
            std::lock_guard<std::mutex> lock(s_stbiImagesMutex);
            s_imageResults.push_back({image, width, height, std::move(texture)});
        }
    });
#endif
}

wgpu::Texture &Texture::texture() {
    return m_texture;
}

void Texture::writeTextures() {
#ifndef __EMSCRIPTEN__
    std::lock_guard<std::mutex> lock(s_stbiImagesMutex);
    if (s_imageResults.empty()) {
        return;
    }

    for (auto &imageResult: s_imageResults) {
        wgpu::ImageCopyTexture destination;
        destination.texture = std::move(imageResult.texture);

        wgpu::TextureDataLayout dataLayout;
        dataLayout.bytesPerRow = imageResult.width * channels;
        dataLayout.rowsPerImage = imageResult.height;

        wgpu::Extent3D size = {static_cast<uint32_t>(imageResult.width), static_cast<uint32_t>(imageResult.height), 1};

        WebGPURenderer::device().GetQueue().WriteTexture(&destination, imageResult.image, imageResult.width * imageResult.height * channels, &dataLayout, &size);

        stbi_image_free(imageResult.image);

        generateMipmap(WebGPURenderer::device(), destination.texture);
    }
    s_imageResults.clear();
#endif
}

}
