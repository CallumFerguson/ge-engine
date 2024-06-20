#include "Texture.hpp"

#include <vector>
#include <stb_image.h>
#include "../Utility/Random.hpp"
#include "Backends/WebGPU/WebGPURenderer.hpp"
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

struct ImageResultMipLevel {
    stbi_uc *image;
    int width;
    int height;
    int mipLevel;
};

struct ImageResult {
    std::vector<ImageResultMipLevel> mipLevels;
    wgpu::Texture texture;
};

static std::mutex s_stbiImagesMutex;
static std::vector<ImageResult> s_imageResults;
#endif

Texture::Texture() : Asset(Random::uuid()) {}

std::function<void()> foo() {
    std::vector<int> vec(5);

    return [&]() {
        std::cout << vec.size() << std::endl;
    };
}

Texture::Texture(const std::string &assetPath) {
    auto assetFile = std::make_shared<std::ifstream>(assetPath, std::ios::binary | std::ios::ate);
    if (!assetFile) {
        std::cerr << "Error: [Texture] Could not open file " << assetPath << " for reading!" << std::endl;
        return;
    }

    std::streamsize fileNumBytes = assetFile->tellg();
    assetFile->seekg(0, std::ios::beg);

    std::string imageType;
    bool hasMipLevels;
    uint32_t imageNumBytes;

    std::filesystem::path path = assetPath;
    std::string extension = path.extension().string();
    if (extension == ".jpeg") {
        extension = ".jpg";
    }

    if (extension == ".getexture") {
        char uuid[37];
        uuid[36] = '\0';
        assetFile->read(uuid, 36);
        m_assetUUID = uuid;

        std::getline(*assetFile, imageType, '\0');

        assetFile->read(reinterpret_cast<char *>(&hasMipLevels), 1);

        assetFile->read(reinterpret_cast<char *>(&imageNumBytes), sizeof(uint32_t));
    } else {
        m_assetUUID = Random::uuid();
        imageType = extension.substr(1);
        hasMipLevels = false;
        imageNumBytes = fileNumBytes;
    }

    if (imageType != "png" && imageType != "jpg") {
        std::cout << "Texture unsupported type " << imageType << std::endl;
        return;
    }

    std::vector<uint8_t> imageData(imageNumBytes);
    assetFile->read(reinterpret_cast<char *>(imageData.data()), imageNumBytes);

    int width, height;
    if (!stbi_info_from_memory(imageData.data(), static_cast<int>(imageData.size()), &width, &height, nullptr)) {
        std::cerr << "Failed to get image info from memory" << std::endl;
        return;
    }

    auto &device = WebGPURenderer::device();

    wgpu::TextureDescriptor textureDescriptor;
    textureDescriptor.size = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1};
    textureDescriptor.mipLevelCount = hasMipLevels ? numMipLevels(textureDescriptor.size) : 1;
    textureDescriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    textureDescriptor.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment;
    m_texture = device.CreateTexture(&textureDescriptor);

#ifdef __EMSCRIPTEN__
    writeTextureJSAsync(device, m_texture, imageData.data(), imageData.size(), false, 0, imageType);

    if (hasMipLevels) {
        uint32_t numLevels = numMipLevels({static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1});
        for (int mipLevel = 1; mipLevel < numLevels; mipLevel++) {
            assetFile->read(reinterpret_cast<char *>(&imageNumBytes), sizeof(uint32_t));

            assetFile->read(reinterpret_cast<char *>(imageData.data()), imageNumBytes);

            writeTextureJSAsync(device, m_texture, imageData.data(), imageNumBytes, false, mipLevel, imageType);
        }
    }
#else
    ThreadPool::instance().queueJob([assetFile = std::move(assetFile), imageData = std::move(imageData), texture = m_texture, hasMipLevels]() mutable {
        ImageResult imageResult;
        imageResult.texture = std::move(texture);

        uint32_t numLevels;
        {
            int width, height;
            stbi_uc *image = stbi_load_from_memory(imageData.data(), static_cast<int>(imageData.size()), &width, &height, nullptr, channels);
            if (image == nullptr) {
                std::cerr << "Failed to load image from memory. mip level: 0" << std::endl;
                return;
            }
            imageResult.mipLevels.push_back({image, width, height, 0});
            numLevels = numMipLevels({static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1});
        }

        if (hasMipLevels) {
            for (uint32_t i = 1; i < numLevels; i++) {
                uint32_t imageNumBytes;
                assetFile->read(reinterpret_cast<char *>(&imageNumBytes), sizeof(uint32_t));

                assetFile->read(reinterpret_cast<char *>(imageData.data()), imageNumBytes);

                int width, height;
                stbi_uc *image = stbi_load_from_memory(imageData.data(), static_cast<int>(imageNumBytes), &width, &height, nullptr, channels);
                if (image == nullptr) {
                    std::cerr << "Failed to load image from memory: mip level: " << i << std::endl;
                    return;
                }

                imageResult.mipLevels.push_back({image, width, height, static_cast<int>(i)});
            }
        }

        {
            std::lock_guard<std::mutex> lock(s_stbiImagesMutex);
            s_imageResults.push_back(std::move(imageResult));
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

    auto &device = WebGPURenderer::device();

    for (auto &imageResult: s_imageResults) {
        for (auto &mipLevel: imageResult.mipLevels) {
            wgpu::ImageCopyTexture destination;
            destination.texture = imageResult.texture;
            destination.mipLevel = mipLevel.mipLevel;

            wgpu::TextureDataLayout dataLayout;
            dataLayout.bytesPerRow = mipLevel.width * channels;
            dataLayout.rowsPerImage = mipLevel.height;

            wgpu::Extent3D size = {static_cast<uint32_t>(mipLevel.width), static_cast<uint32_t>(mipLevel.height), 1};

            device.GetQueue().WriteTexture(&destination, mipLevel.image, mipLevel.width * mipLevel.height * channels, &dataLayout, &size);

            stbi_image_free(mipLevel.image);
        }
    }
    s_imageResults.clear();
#endif
}

wgpu::TextureView &Texture::cachedTextureView() {
    if (!m_textureView) {
        m_textureView = m_texture.CreateView();
    }
    return m_textureView;
}

}
