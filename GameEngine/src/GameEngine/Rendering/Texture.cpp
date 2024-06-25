#include "Texture.hpp"

#include <unordered_set>
#include <vector>
#include <stb_image.h>
#include "../Utility/Random.hpp"
#include "Backends/WebGPU/WebGPURenderer.hpp"
#include "Backends/WebGPU/generateMipmapWebGPU.hpp"
#include "../Utility/TimingHelper.hpp"

#ifdef __EMSCRIPTEN__

#include "../Utility/emscriptenUtility.hpp"

#else

#include <mutex>
#include <half.hpp>
#include "../Utility/ThreadPool.hpp"

using half_float::half;

#endif

namespace GameEngine {

static const int channels = 4;

#ifndef __EMSCRIPTEN__

struct ImageResultMipLevel {
    stbi_uc *image;
    float *floatImage;
    int width;
    int height;
    int channelByteSize;
    int mipLevel;
};

struct ImageResult {
    std::vector<ImageResultMipLevel> mipLevels;
    wgpu::Texture texture;
    size_t readyStateIndex;
};

static std::mutex s_stbiImagesMutex;
static std::vector<ImageResult> s_imageResults;
#endif

struct TextureReadyState {
    bool ready;
    std::function<void()> readyCallback;
};

static std::vector<TextureReadyState> s_textureReadyStates;

Texture::Texture() : Asset(Random::uuid()) {}

std::function<void()> foo() {
    std::vector<int> vec(5);

    return [&]() {
        std::cout << vec.size() << std::endl;
    };
}

Texture::Texture(const std::string &assetPath, wgpu::TextureFormat requestedFormat, bool forceMipLevels, wgpu::TextureUsage extraFlags) {
    auto assetFile = std::make_shared<std::ifstream>(assetPath, std::ios::binary | std::ios::ate);
    if (!assetFile) {
        std::cerr << "Error: [Texture] Could not open file " << assetPath << " for reading!" << std::endl;
        return;
    }

    uint32_t fileNumBytes = assetFile->tellg();
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

    static const std::unordered_set<std::string> supportedFileTypes = {"png", "jpg", "hdr"};
    if (!supportedFileTypes.contains(imageType)) {
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
    m_size = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1};
    textureDescriptor.size = m_size;
    textureDescriptor.mipLevelCount = (hasMipLevels || forceMipLevels) ? numMipLevels(textureDescriptor.size) : 1;
    textureDescriptor.format = imageType == "hdr" ? wgpu::TextureFormat::RGBA16Float : wgpu::TextureFormat::RGBA8Unorm;
#ifndef __EMSCRIPTEN__
    if (requestedFormat != wgpu::TextureFormat::Undefined) {
        textureDescriptor.format = requestedFormat;
    }
#endif
    textureDescriptor.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment | extraFlags;
    m_texture = device.CreateTexture(&textureDescriptor);

    m_readyStateIndex = s_textureReadyStates.size();
    s_textureReadyStates.push_back({false, nullptr});

#ifdef __EMSCRIPTEN__
    writeTextureJSAsync(device, m_texture, imageData.data(), imageData.size(), false, 0, imageType, static_cast<int>(m_readyStateIndex));

    if (hasMipLevels) {
        uint32_t numLevels = numMipLevels({static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1});
        for (int mipLevel = 1; mipLevel < numLevels; mipLevel++) {
            assetFile->read(reinterpret_cast<char *>(&imageNumBytes), sizeof(uint32_t));

            assetFile->read(reinterpret_cast<char *>(imageData.data()), imageNumBytes);

            writeTextureJSAsync(device, m_texture, imageData.data(), imageNumBytes, false, mipLevel, imageType, static_cast<int>(m_readyStateIndex));
        }
    }
#else
    ThreadPool::instance().queueJob([
                                            assetFile = std::move(assetFile),
                                            imageData = std::move(imageData),
                                            texture = m_texture,
                                            hasMipLevels,
                                            imageType = std::move(imageType),
                                            readyStateIndex = m_readyStateIndex,
                                            textureFormat = textureDescriptor.format
                                    ]() mutable {
        ImageResult imageResult;
        imageResult.texture = std::move(texture);
        imageResult.readyStateIndex = readyStateIndex;

        uint32_t numLevels;
        {
            if (imageType == "hdr") {
                int width, height;
                float *floatImage = stbi_loadf_from_memory(imageData.data(), static_cast<int>(imageData.size()), &width, &height, nullptr, channels);
                if (floatImage == nullptr) {
                    std::cerr << "Failed to load image from memory. mip level: 0" << std::endl;
                    return;
                }

                int channelByteSize = 1;
                if (textureFormat == wgpu::TextureFormat::RGBA16Float) {
                    channelByteSize = 2;
                    half *halfImage = reinterpret_cast<half *>(floatImage);
                    // float image is twice the size needed for the image as halfs, so just reuse the memory allocated by stb
                    for (int i = 0; i < width * height * channels; i++) {
                        float value = floatImage[i];
                        // prevent float from getting turned into infinity when represented as a half
                        if (value > 65500) {
                            value = 65500;
                        }
                        halfImage[i] = value;
                    }
                } else if (textureFormat == wgpu::TextureFormat::RGBA32Float) {
                    channelByteSize = 4;
                } else {
                    std::cout << "bad texture format for hdr texture" << std::endl;
                }

                imageResult.mipLevels.push_back({nullptr, floatImage, width, height, channelByteSize, 0});
                numLevels = numMipLevels({static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1});
            } else {
                int width, height;
                stbi_uc *image = stbi_load_from_memory(imageData.data(), static_cast<int>(imageData.size()), &width, &height, nullptr, channels);
                if (image == nullptr) {
                    std::cerr << "Failed to load image from memory. mip level: 0" << std::endl;
                    return;
                }
                imageResult.mipLevels.push_back({image, nullptr, width, height, 1, 0});
                numLevels = numMipLevels({static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1});
            }
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

                imageResult.mipLevels.push_back({image, nullptr, width, height, 1, static_cast<int>(i)});
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
            bool imageIsAsHalfs = mipLevel.floatImage != nullptr;

            wgpu::ImageCopyTexture destination;
            destination.texture = imageResult.texture;
            destination.mipLevel = mipLevel.mipLevel;

            wgpu::TextureDataLayout dataLayout;
            dataLayout.bytesPerRow = mipLevel.width * channels * mipLevel.channelByteSize;
            dataLayout.rowsPerImage = mipLevel.height;

            wgpu::Extent3D size = {static_cast<uint32_t>(mipLevel.width), static_cast<uint32_t>(mipLevel.height), 1};

            if (imageIsAsHalfs) {
                device.GetQueue().WriteTexture(&destination, mipLevel.floatImage, mipLevel.width * mipLevel.height * channels * mipLevel.channelByteSize, &dataLayout, &size);
                stbi_image_free(mipLevel.floatImage);
            } else {
                device.GetQueue().WriteTexture(&destination, mipLevel.image, mipLevel.width * mipLevel.height * channels * mipLevel.channelByteSize, &dataLayout, &size);
                stbi_image_free(mipLevel.image);
            }
        }
        setTextureReady(static_cast<int>(imageResult.readyStateIndex));
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

bool Texture::ready() {
    return s_textureReadyStates[m_readyStateIndex].ready;
}

void Texture::setReadyCallback(std::function<void()> readyCallback) {
    s_textureReadyStates[m_readyStateIndex].readyCallback = std::move(readyCallback);
    if (s_textureReadyStates[m_readyStateIndex].ready) {
        s_textureReadyStates[m_readyStateIndex].readyCallback();
    }
}

void Texture::setTextureReady(int readyStateIndex) {
    auto &readyState = s_textureReadyStates[readyStateIndex];
    readyState.ready = true;
    if (readyState.readyCallback) {
        readyState.readyCallback();
    }
}

const wgpu::Extent3D &Texture::size() const {
    return m_size;
}


}
