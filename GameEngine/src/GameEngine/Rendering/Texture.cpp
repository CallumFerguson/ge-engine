#include "Texture.hpp"

#include <vector>
#include <nlohmann/json.hpp>
#include <stb_image.h>
#include "../Utility/Random.hpp"
#include "Backends/WebGPU/WebGPURenderer.hpp"

//#ifdef __EMSCRIPTEN__
//
//#include "../Utility/emscriptenUtility.hpp"
//
//#else
//
//#include <stb_image.h>
//
//#endif

namespace GameEngine {

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

    int channels = 4;
    int width, height;
    unsigned char *image = stbi_load_from_memory(imageData.data(), static_cast<int>(imageData.size()), &width, &height, nullptr, 4);
    if (image == nullptr) {
        std::cerr << "Failed to load image from memory" << std::endl;
        return;
    }

    auto &device = WebGPURenderer::device();

    wgpu::TextureDescriptor textureDescriptor;
    textureDescriptor.size = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1};
    textureDescriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    textureDescriptor.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst; // | wgpu::TextureUsage::RenderAttachment;
    m_texture = device.CreateTexture(&textureDescriptor);

//    writeTextureJS(device, m_texture, imageData);

    wgpu::ImageCopyTexture destination;
    destination.texture = m_texture;

    wgpu::TextureDataLayout dataLayout;
    dataLayout.bytesPerRow = width * channels;
    dataLayout.rowsPerImage = height;

    device.GetQueue().WriteTexture(&destination, image, width * height * channels, &dataLayout, &textureDescriptor.size);

    stbi_image_free(image);
}

wgpu::Texture &Texture::texture() {
    return m_texture;
}

}
