#include "texturePacker.hpp"

#include <iostream>
#include <stb_image_write.h>
#include "GameEngine.hpp"

namespace GameEngineTools {

static std::vector<uint8_t> s_stbImageWriteBuffer;

void writeImageDataToFile(void *context, void *data, int size) {
    auto byteData = reinterpret_cast<const char *>(data);
    s_stbImageWriteBuffer.insert(s_stbImageWriteBuffer.end(), byteData, byteData + size);
}

bool writeGLTFTextureImageFile(const tinygltf::Image &image, const std::string &name, const std::filesystem::path &outputFilePath, const std::string &textureUUID) {
    auto path = outputFilePath / (name + ".getexture");

    TimingHelper time("packed texture " + name);

    std::ofstream outputFile(path, std::ios::out | std::ios::binary);
    if (!outputFile) {
        std::cerr << "Error: Could not open file for writing!" << std::endl;
        return false;
    }

    outputFile << textureUUID;

    // PNG ends up with files that are larger than the gltf source pngs and paint.net pngs, so maybe these should be copied over directly if possible
//    stbi_write_png_to_func(writeImageDataToFile, &outputFile, image.width, image.height, image.component, image.image.data(), image.width * image.component);

    if (image.component != 4) {
        // TODO: this might not even be needed. if it is, there should be some way to make it not needed
        std::cout << "writeGLTFTextureImageFile image.component should be 4" << std::endl;
    }

    bool hasTransparency = false;
    for (size_t i = 0; i < image.image.size(); i += 4) {
        if (image.image[i + 3] != 255) {
            hasTransparency = true;
            break;
        }
    }

    std::string imageType = hasTransparency ? "png" : "jpg";
    outputFile.write(imageType.c_str(), imageType.size() + 1);

    auto &device = GameEngine::WebGPURenderer::device();

    wgpu::TextureDescriptor textureDescriptor;
    textureDescriptor.size = {static_cast<uint32_t>(image.width), static_cast<uint32_t>(image.height), 1};
    textureDescriptor.mipLevelCount = GameEngine::numMipLevels(textureDescriptor.size);
    textureDescriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    textureDescriptor.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
    auto texture = device.CreateTexture(&textureDescriptor);

    wgpu::ImageCopyTexture destination;
    destination.texture = std::move(texture);

    wgpu::TextureDataLayout dataLayout;
    dataLayout.bytesPerRow = image.width * image.component;
    dataLayout.rowsPerImage = image.height;

    wgpu::Extent3D size = {static_cast<uint32_t>(image.width), static_cast<uint32_t>(image.height), 1};

    device.GetQueue().WriteTexture(&destination, image.image.data(), image.image.size(), &dataLayout, &size);

    GameEngine::generateMipmap(device, destination.texture);

    wgpu::BufferDescriptor descriptor;
    descriptor.size = image.image.size();
    descriptor.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead;
    auto readBackBuffer = device.CreateBuffer(&descriptor);

    bool hasMipLevels = true;
    outputFile.write(reinterpret_cast<char *>(&hasMipLevels), 1);

    for (uint32_t level = 0; level < textureDescriptor.mipLevelCount; level++) {
        uint32_t mipWidth = std::max(1u, size.width >> level);
        uint32_t mipHeight = std::max(1u, size.height >> level);
        uint32_t mipSize = mipWidth * mipHeight * 4;

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ImageCopyTexture src = {};
        src.texture = destination.texture;
        src.mipLevel = level;

        uint32_t bytesPerRow = mipWidth * 4;
        uint32_t paddedBytesPerRow = std::max(256u, bytesPerRow);

        wgpu::ImageCopyBuffer dst = {};
        dst.buffer = readBackBuffer;
        dst.layout.offset = 0;
        dst.layout.bytesPerRow = paddedBytesPerRow;
        dst.layout.rowsPerImage = mipHeight;

        wgpu::Extent3D extent = {mipWidth, mipHeight, 1};
        encoder.CopyTextureToBuffer(&src, &dst, &extent);

        wgpu::CommandBuffer commands = encoder.Finish();
        device.GetQueue().Submit(1, &commands);

        readBackBuffer.MapAsync(wgpu::MapMode::Read, 0, mipSize, [](WGPUBufferMapAsyncStatus status, void *userData) {
            if (status != WGPUBufferMapAsyncStatus_Success) {
                std::cout << "Failed to map buffer for reading." << std::endl;
            }
        }, nullptr);

        while (readBackBuffer.GetMapState() != wgpu::BufferMapState::Mapped) {
            device.Tick();
        }

        auto *data = reinterpret_cast<const uint8_t *>(readBackBuffer.GetConstMappedRange(0, mipSize));
        if (!data) {
            std::cout << "no mapped range data!" << std::endl;
            return false;
        }

        const uint8_t *dataWithoutPadding = data;
        std::vector<uint8_t> dataWithoutPaddingVector;

        if (bytesPerRow < paddedBytesPerRow) {
            dataWithoutPaddingVector.resize(bytesPerRow * mipHeight);
            for (uint32_t row = 0; row < mipHeight; row++) {
                for (uint32_t column = 0; column < bytesPerRow; column++) {
                    dataWithoutPaddingVector[row * bytesPerRow + column] = data[row * paddedBytesPerRow + column];
                }
            }
            dataWithoutPadding = dataWithoutPaddingVector.data();
        }

        if (hasTransparency) {
            stbi_write_png_to_func(writeImageDataToFile, &outputFile, static_cast<int>(mipWidth), static_cast<int>(mipHeight), image.component, dataWithoutPadding, static_cast<int>(bytesPerRow));
        } else {
            stbi_write_jpg_to_func(writeImageDataToFile, &outputFile, static_cast<int>(mipWidth), static_cast<int>(mipHeight), image.component, dataWithoutPadding, 90);
        }

        uint32_t imageNumBytes = s_stbImageWriteBuffer.size();
        outputFile.write(reinterpret_cast<const char *>(&imageNumBytes), sizeof(uint32_t));

        outputFile.write(reinterpret_cast<const char *>(s_stbImageWriteBuffer.data()), s_stbImageWriteBuffer.size());

        s_stbImageWriteBuffer.clear();

        readBackBuffer.Unmap();
    }

    return hasTransparency;
}

void writeFakeTexture(const uint8_t *data, const std::string &name, const std::filesystem::path &outputFilePath, const std::string &textureUUID) {
    auto path = outputFilePath / (name + ".getexture");

    TimingHelper time("packed texture " + name);

    std::ofstream outputFile(path, std::ios::out | std::ios::binary);
    if (!outputFile) {
        std::cerr << "Error: Could not open file for writing!" << std::endl;
        return;
    }

    outputFile << textureUUID;

    std::string imageType = "png";
    outputFile.write(imageType.c_str(), imageType.size() + 1);

    bool hasMipLevels = false;
    outputFile.write(reinterpret_cast<char *>(&hasMipLevels), 1);

    stbi_write_png_to_func(writeImageDataToFile, &outputFile, 1, 1, 4, data, 4);

    uint32_t imageNumBytes = s_stbImageWriteBuffer.size();
    outputFile.write(reinterpret_cast<const char *>(&imageNumBytes), sizeof(uint32_t));

    outputFile.write(reinterpret_cast<const char *>(s_stbImageWriteBuffer.data()), s_stbImageWriteBuffer.size());

    s_stbImageWriteBuffer.clear();
}

}
